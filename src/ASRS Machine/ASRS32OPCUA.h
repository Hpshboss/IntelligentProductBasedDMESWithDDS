/// @author Alexander Rykovanov 2013
/// @email rykovanov.as@gmail.com
/// @brief Remote Computer implementaion.
/// @license GNU LGPL
///
/// Distributed under the GNU LGPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/lgpl.html)
///

#include <opc/ua/client/client.h>
#include <opc/ua/node.h>
#include <opc/ua/subscription.h>

#include <opc/common/logger.h>

#include <iostream>
#include <string>
#include <stdexcept>
#include <thread>

using namespace OpcUa;

namespace festoLab
{
    enum MachineStates
    {
        READY,
        BUSY, 
        UNKNOWN
    };

    class ASRS32OpcuaAgent
    {
    public:
        ASRS32OpcuaAgent(std::shared_ptr<spdlog::logger> logger) : Logger(logger)
        {
            try
            {
                Logger->info("Connecting to: {}", endpoint);
                client.Connect(endpoint);

                // 初始化部分節點以便降低函式套用的延遲時間
                automaticNode = client.GetNode(NodeId("\"dbVar\".\"OpMode\".\"Auto\".\"xAct\"", 3));
                cycleEndNode = client.GetNode(NodeId("\"dbVar\".\"OpMode\".\"CycleEnd\".\"xAct\"", 3));

                arriving1SensorNode = client.GetNode(NodeId("\"xG1_BG24\"", 3));
                arrival1SensorNode = client.GetNode(NodeId("\"xG1_BG22\"", 3));

                arriving2SensorNode = client.GetNode(NodeId("\"xG1_BG34\"", 3));
                arrival2SensorNode = client.GetNode(NodeId("\"xG1_BG32\"", 3));

                busyStateNode = client.GetNode(NodeId("\"dbAppIF\".\"Out\".\"xBusy\"", 3));
                readyStateNode = client.GetNode(NodeId("\"dbAppIF\".\"Out\".\"xReady\"", 3));
            }
            catch (const std::exception & exc)
            {
                Logger->error("Error in constructor: {}", exc.what());
            }
            catch (...)
            {
                Logger->error("Unknown error in constructor.");
            }
        };

        virtual ~ASRS32OpcuaAgent() 
        {
            try
            {
                Logger->info("Disconnecting");
                client.Disconnect();
                Logger->flush();
            }
            catch (const std::exception & exc)
            {
                Logger->error("Error in disconstructor: {}", exc.what());
            }
            catch (...)
            {
                Logger->error("Unknown error in disconstructor.");
            }
        };

        /*************************Transition Functions*************************/
        void addTransition(short transitionIndex, short startConditionValue, int functionValue, int partNumberValue, short endConditionValue)
        {
            startCoditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondIn\"", 3));
            startCoditionNode.SetValue(Variant(startConditionValue));

            applicationExecuteNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"xAppEn\"", 3));
            applicationExecuteNode.SetValue(Variant(false));

            functionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"adiPar\"[0]", 3));
            functionNode.SetValue(Variant(functionValue));

            partNumberNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"adiPar\"[1]", 3));
            partNumberNode.SetValue(Variant(partNumberValue));

            endConditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondOut\"", 3));
            endConditionNode.SetValue(Variant(endConditionValue));
        };

        void deleteTransition(short transitionIndex)
        {
            startCoditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondIn\"", 3));
            startCoditionNode.SetValue(Variant((short)0));

            applicationExecuteNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"xAppEn\"", 3));
            applicationExecuteNode.SetValue(Variant(false));

            functionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"adiPar\"[0]", 3));
            functionNode.SetValue(Variant((int)0));

            partNumberNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"adiPar\"[1]", 3));
            partNumberNode.SetValue(Variant((int)0));

            endConditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondOut\"", 3));
            endConditionNode.SetValue(Variant((short)0));
        };

        void transitionExecutable(short transitionIndex, bool executable)
        {
            applicationExecuteNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"xAppEn\"", 3));
            applicationExecuteNode.SetValue(Variant(executable));
        };


        /*************************RFID Functions*************************/
        short readStopper1Rfid()
        {
            rfid1DataReadNode = client.GetNode(NodeId("\"dbRfidCntr\".\"ID1\".\"xRead\"", 3));
            rfid1DataReadNode.SetValue(Variant(true));
            rfid1DataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID1\".\"iCode\"", 3));
            return rfid1DataNode.GetValue().As<short>();
        };

        short readStopper2Rfid()
        {
            rfid2DataReadNode = client.GetNode(NodeId("\"dbRfidCntr\".\"ID1\".\"xRead\"", 3));
            rfid2DataReadNode.SetValue(Variant(true));
            rfid2DataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID1\".\"iCode\"", 3));
            return rfid1DataNode.GetValue().As<short>();
        };

        void writeStopper1Rfid(short rfidData) 
        {
            rfid1DataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID1\".\"iCode\"", 3));
            rfid1DataNode.SetValue(Variant(rfidData));
            rfid1DataWriteNode = client.GetNode(NodeId("\"dbRfidCtrl\".\"ID1\".\"xWrite\"", 3));
            rfid1DataWriteNode.SetValue(Variant(true));
        };

        void writeStopper2Rfid(short rfidData) 
        {
            rfid2DataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID2\".\"iCode\"", 3));
            rfid2DataNode.SetValue(Variant(rfidData));
            rfid2DataWriteNode = client.GetNode(NodeId("\"dbRfidCtrl\".\"ID2\".\"xWrite\"", 3));
            rfid2DataWriteNode.SetValue(Variant(true));
        };


        /*************************Operation Mode Functions*************************/
        bool automatic() 
        {
            automaticNode.SetValue(Variant(true));
            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                return automaticNode.GetValue().As<bool>();
            }
            catch (const std::exception & exc)
            {
                Logger->error("Error in automatic: {}", exc.what());
                return false;
            }
        }

        bool cycleEnd()
        {
            cycleEndNode.SetValue(Variant(true));
            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                return cycleEndNode.GetValue().As<bool>();
            }
            catch (const std::exception & exc)
            {
                Logger->error("Error in automatic: {}", exc.what());
                return false;
            }
        };

        /*************************Monitor Carrier Arrival Functions*************************/
        // Stopper 1
        bool readStopper1ArrivingSensor() 
        {
            try
            {
                return arriving1SensorNode.GetValue().As<bool>();
            }
            catch (const std::exception & exc)
            {
                Logger->error("Error in readStopper1ArrivingSensor: {}", exc.what());
                return false;
            }
        };

        bool readStopper1ArrivalSensor() 
        {
            // Logger->debug("Read Arrival Sensor");
            try
            {
                return arrival1SensorNode.GetValue().As<bool>();
            }
            catch (const std::exception & exc)
            {
                Logger->error("Error in readStopper1ArrivalSensor: {}", exc.what());
                return false;
            }
        };

        void keepMonitoringCarrierStopper1(bool *isStop) 
        {
            Logger->debug("Keep Monitoring Carrier");
            Variant cycleEndVariant(true);

            while (!readStopper1ArrivingSensor()) 
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            while (!readStopper1ArrivalSensor()) 
            {
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            // cycleEnd(); 避免套函式延誤時間
            cycleEndNode.SetValue(cycleEndVariant);
            *isStop = true;
        };

        void monitorCarrierArrivalThenStopStopper1(bool *isStopp) 
        {
            std::thread monitorCarrierArrivalStopper1Thread(&ASRS32OpcuaAgent::keepMonitoringCarrierStopper1, this, isStopp);
            Logger->debug("Monitoring Carrier Arrival Then Stop");
            monitorCarrierArrivalStopper1Thread.detach();
        };

        // Stopper 2
        bool readStopper2ArrivingSensor() 
        {
            try
            {
                return arriving1SensorNode.GetValue().As<bool>();
            }
            catch (const std::exception & exc)
            {
                Logger->error("Error in readAStopper2rrivingSensor: {}", exc.what());
                return false;
            }
        };

        bool readStopper2ArrivalSensor() 
        {
            // Logger->debug("Read Arrival Sensor");
            try
            {
                return arrival1SensorNode.GetValue().As<bool>();
            }
            catch (const std::exception & exc)
            {
                Logger->error("Error in readStopper2ArrivalSensor: {}", exc.what());
                return false;
            }
        };

        void keepMonitoringCarrierStopper2(bool *isStop) 
        {
            Logger->debug("Keep Monitoring Carrier");
            Variant cycleEndVariant(true);

            while (!readStopper2ArrivingSensor()) 
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            while (!readStopper2ArrivalSensor()) 
            {
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            // cycleEnd(); 避免套函式延誤時間
            cycleEndNode.SetValue(cycleEndVariant);
            *isStop = true;
        };

        void monitorCarrierArrivalThenStopStopper2(bool *isStopp) 
        {
            std::thread monitorCarrierArrivalStopper2Thread(&ASRS32OpcuaAgent::keepMonitoringCarrierStopper2, this, isStopp);
            Logger->debug("Monitoring Carrier Arrival Then Stop");
            monitorCarrierArrivalStopper2Thread.detach();
        };

        /*************************Monitor Carrier Arrival Functions*************************/
        MachineStates readMachineState()
        {
            if ( readyStateNode.GetValue().As<bool>() ) { return MachineStates::READY; }

            if ( busyStateNode.GetValue().As<bool>() ) { return MachineStates::BUSY; }

            return MachineStates::UNKNOWN;
        }

        void keepMonitoringMachineState(MachineStates* machineState)
        {
            while (true)
            {
                *machineState = readMachineState();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        void monitorMachineState(MachineStates* machineState)
        {
            std::thread monitorMachineStateThread(&ASRS32OpcuaAgent::keepMonitoringMachineState, this, machineState);
            Logger->debug("Monitoring Machine State");
            monitorMachineStateThread.detach();
        }

    protected:
        std::string endpoint = "opc.tcp://172.21.3.1:4840/";
        Common::Logger::SharedPtr Logger;
        OpcUa::UaClient client;

        // transition nodes
        Node startCoditionNode;
        Node applicationExecuteNode;
        Node functionNode;
        Node partNumberNode;
        Node endConditionNode;

        // RFID nodes
        Node rfid1DataNode;     // 儲存RFID值之節點
        Node rfid1DataReadNode;     // 觸發讀取功能之節點
        Node rfid1DataWriteNode;    // 觸發寫入功能之節點
        Node rfid2DataNode;
        Node rfid2DataReadNode;
        Node rfid2DataWriteNode;

        // Operation mode nodes
        Node automaticNode;
        Node cycleEndNode;

        // Monitor Carrier
        Node arriving1SensorNode;   // 慢速偵測小車之節點，以觸發快速偵測小車之節點(arrival1SensorNode)
        Node arrival1SensorNode;    // 快速偵測小車之節點
        Node arriving2SensorNode;
        Node arrival2SensorNode;

        // Machine State nodes
        Node busyStateNode;
        Node readyStateNode;
    };
}