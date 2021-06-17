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

    class MPressOpcuaAgent
    {
    public:
        MPressOpcuaAgent(std::shared_ptr<spdlog::logger> logger) : Logger(logger)
        {
            try
            {
                Logger->info("Connecting to: {}", endpoint);
                client.Connect(endpoint);

                // 初始化部分節點以便降低函式套用的延遲時間
                automaticNode = client.GetNode(NodeId("\"dbVar\".\"OpMode\".\"Auto\".\"xAct\"", 3));
                cycleEndNode = client.GetNode(NodeId("\"dbVar\".\"OpMode\".\"CycleEnd\".\"xAct\"", 3));

                arrivingSensorNode = client.GetNode(NodeId("\"xG1_BG24\"", 3));
                arrivalSensorNode = client.GetNode(NodeId("\"xG1_BG22\"", 3));

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

        virtual ~MPressOpcuaAgent() 
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
        void addTransition(short transitionIndex, short startConditionValue, int pressForceValue, int pressTimeValue, short endConditionValue)
        {
            startCoditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondIn\"", 3));
            startCoditionNode.SetValue(Variant(startConditionValue));

            applicationExecuteNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"xAppEn\"", 3));
            applicationExecuteNode.SetValue(Variant(false));

            pressForceNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"adiPar\"[0]", 3));
            pressForceNode.SetValue(Variant(pressForceValue));

            pressTimeNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"adiPar\"[1]", 3));
            pressTimeNode.SetValue(Variant(pressTimeValue));

            endConditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondOut\"", 3));
            endConditionNode.SetValue(Variant(endConditionValue));
        };

        void deleteTransition(short transitionIndex)
        {
            startCoditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondIn\"", 3));
            startCoditionNode.SetValue(Variant(short(0)));

            applicationExecuteNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"xAppEn\"", 3));
            applicationExecuteNode.SetValue(Variant(false));

            pressForceNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"adiPar\"[0]", 3));
            pressForceNode.SetValue(Variant((int)0));

            pressTimeNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"adiPar\"[1]", 3));
            pressTimeNode.SetValue(Variant((int)0));

            endConditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondOut\"", 3));
            endConditionNode.SetValue(Variant((short)0));
        };

        void transitionExecutable(short transitionIndex, bool executable)
        {
            applicationExecuteNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"xAppEn\"", 3));
            applicationExecuteNode.SetValue(Variant(executable));
        };

        /*************************RFID Functions*************************/
        short readRfid()
        {
            rfidDataReadNode = client.GetNode(NodeId("\"dbRfidCntr\".\"ID1\".\"xRead\"", 3));
            rfidDataReadNode.SetValue(Variant(true));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            rfidDataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID1\".\"iCode\"", 3));
            return rfidDataNode.GetValue().As<short>();
        };

        void writeRfid(short rfidData) 
        {
            rfidDataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID1\".\"iCode\"", 3));
            rfidDataNode.SetValue(Variant(rfidData));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            rfidDataWriteNode = client.GetNode(NodeId("\"dbRfidCntr\".\"ID1\".\"xWrite\"", 3));
            rfidDataWriteNode.SetValue(Variant(true));
        };

        /*************************Operation Mode Functions*************************/
        bool automatic()
        {
            automaticNode.SetValue(Variant(true));
            try
            {
                return automaticNode.GetValue().As<bool>();
            }
            catch (const std::exception & exc)
            {
                Logger->error("Error in automatic: {}", exc.what());
                return false;
            }
        }

        void cycleEnd()
        {
            cycleEndNode.SetValue(Variant(true));
        };

        /*************************Monitor Carrier Arrival Functions*************************/
        bool readArrivingSensor() 
        {
            std::mutex m;
            std::condition_variable cv;
            bool retValue;

            std::thread t([&cv, &retValue](Node* arrivingSensorNode)
            {
                try 
                {
                    retValue = arrivingSensorNode->GetValue().As<bool>();
                }
                catch (const std::exception & exc) {}
                cv.notify_one();
            }, &arrivingSensorNode);

            t.detach();
            
            std::unique_lock<std::mutex> l(m);
            if (cv.wait_for(l, std::chrono::milliseconds(100)) == std::cv_status::timeout)
            {
                Logger->error("Error in readArrivingSensor: Timeout");
                return false;
            }

            return retValue;
        };

        bool readArrivalSensor() 
        {
            std::mutex m;
            std::condition_variable cv;
            bool retValue;

            std::thread t([&cv, &retValue](Node* arrivalSensorNode)
            {
                try
                {
                    retValue = arrivalSensorNode->GetValue().As<bool>();
                }
                catch (const std::exception & exc) {}
                cv.notify_one();
            }, &arrivalSensorNode);

            t.detach();
            
            std::unique_lock<std::mutex> l(m);
            if (cv.wait_for(l, std::chrono::milliseconds(100)) == std::cv_status::timeout)
            {
                Logger->error("Error in readArrivalSensor: Timeout");
                return false;
            }

            return retValue;
        };

        void keepMonitoringCarrier(bool *isStop) {
            Logger->debug("Keep Monitoring Carrier");
            Variant cycleEndVariant(true);

            int i = 0;
            while (!readArrivingSensor()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (i % 10 == 0) { Logger->debug("reading stopper arriving sensor"); }
                i++;
            }

            while (!readArrivalSensor()) {
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            // cycleEnd(); 避免套函式延誤時間
            cycleEndNode.SetValue(cycleEndVariant);
            *isStop = true;
        };

        void monitorCarrierArrivalThenStop(bool *isStopp) {
            std::thread monitorCarrierArrivalThread(&MPressOpcuaAgent::keepMonitoringCarrier, this, isStopp);
            Logger->debug("Monitoring Carrier Arrival Then Stop");
            monitorCarrierArrivalThread.detach();
        };

        /*************************Monitor Carrier Arrival Functions*************************/
        MachineStates readMachineState()
        {
            if ( readyStateNode.GetValue().As<bool>() ) { return MachineStates::READY; }

            if ( busyStateNode.GetValue().As<bool>() ) { return MachineStates::BUSY; }

            return MachineStates::UNKNOWN;
        };

        void keepMonitoringMachineState(MachineStates* machineState)
        {
            while (true)
            {
                *machineState = readMachineState();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        };

        void monitorMachineState(MachineStates* machineState)
        {
            std::thread monitorMachineStateThread(&MPressOpcuaAgent::keepMonitoringMachineState, this, machineState);
            Logger->debug("Monitoring Machine State");
            monitorMachineStateThread.detach();
        };

    protected:
        std::string endpoint = "opc.tcp://172.21.2.1:4840/";
        Common::Logger::SharedPtr Logger;
        OpcUa::UaClient client;

        // transition nodes
        Node startCoditionNode;
        Node applicationExecuteNode;
        Node pressForceNode;
        Node pressTimeNode;
        Node endConditionNode;

        // RFID nodes
        Node rfidDataNode;      // 儲存RFID值之節點
        Node rfidDataReadNode;      // 觸發讀取功能之節點
        Node rfidDataWriteNode;     // 觸發寫入功能之節點

        // Operation mode nodes
        Node automaticNode;
        Node cycleEndNode;

        // Monitor Carrier
        Node arrivingSensorNode;    // 慢速偵測小車之節點，以觸發快速偵測小車之節點(arrivalSensorNode)
        Node arrivalSensorNode;     // 快速偵測小車之節點

        // Machine State nodes
        Node busyStateNode;
        Node readyStateNode;
    };
}