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
    class MagBack
    {
    public:
        MagBack(std::shared_ptr<spdlog::logger> logger) : Logger(logger)
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

        virtual ~MagBack() 
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
        void addTransition(short transitionIndex, short startConditionValue, short endConditionValue)
        {
            startCoditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondIn\"", 3));
            startCoditionNode.SetValue(Variant(startConditionValue));

            applicationExecuteNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"xAppEn\"", 3));
            applicationExecuteNode.SetValue(Variant(false));

            endConditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondOut\"", 3));
            endConditionNode.SetValue(Variant(endConditionValue));
        };

        void deleteTransition(short transitionIndex)
        {
            startCoditionNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"iCondIn\"", 3));
            startCoditionNode.SetValue(Variant((short)0));

            applicationExecuteNode = client.GetNode(NodeId("\"dbAppPar\".\"Conditions\"["+std::to_string(transitionIndex)+"].\"xAppEn\"", 3));
            applicationExecuteNode.SetValue(Variant(false));

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
            rfidDataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID1\".\"iCode\"", 3));
            return rfidDataNode.GetValue().As<short>();
        };

        void writeRfid(short rfidData) 
        {
            rfidDataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID1\".\"iCode\"", 3));
            rfidDataNode.SetValue(Variant(rfidData));
            rfidDataWriteNode = client.GetNode(NodeId("\"dbRfidCntr\".\"ID1\".\"xWrite\"", 3));
            rfidDataWriteNode.SetValue(Variant(true));
        };

        /*************************Operation Mode Functions*************************/
        void automatic()
        {
            automaticNode.SetValue(Variant(true));
        }

        void cycleEnd()
        {
            cycleEndNode.SetValue(Variant(true));
        };

        /*************************Monitor Carrier Arrival Functions*************************/
        bool readArrivingSensor() {
            return arrivingSensorNode.GetValue().As<bool>();
        };

        bool readArrivalSensor() {
            // Logger->debug("Read Arrival Sensor");
            return arrivalSensorNode.GetValue().As<bool>();
        };

        void keepMonitoringCarrier(bool *isStop) {
            Logger->debug("Keep Monitoring Carrier");
            Variant cycleEndVariant(true);

            while (!readArrivingSensor()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            while (!readArrivalSensor()) {
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            // cycleEnd(); 避免套函式延誤時間
            cycleEndNode.SetValue(cycleEndVariant);
            *isStop = true;
        };

        void monitorCarrierArrivalThenStop(bool *isStopp) {
            std::thread monitorCarrierArrivalThread(&MagBack::keepMonitoringCarrier, this, isStopp);
            Logger->debug("Monitoring Carrier Arrival Then Stop");
            monitorCarrierArrivalThread.detach();
        };


    protected:
        std::string endpoint = "opc.tcp://172.21.1.1:4840/";
        Common::Logger::SharedPtr Logger;
        OpcUa::UaClient client;
        
        // transition nodes
        Node startCoditionNode;
        Node applicationExecuteNode;
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
        
    };


    class MPress
    {
    public:
        MPress(std::shared_ptr<spdlog::logger> logger) : Logger(logger)
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

        virtual ~MPress() 
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
            rfidDataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID1\".\"iCode\"", 3));
            return rfidDataNode.GetValue().As<short>();
        };

        void writeRfid(short rfidData) 
        {
            rfidDataNode = client.GetNode(NodeId("\"dbRfidData\".\"ID1\".\"iCode\"", 3));
            rfidDataNode.SetValue(Variant(rfidData));
            rfidDataWriteNode = client.GetNode(NodeId("\"dbRfidCntr\".\"ID1\".\"xWrite\"", 3));
            rfidDataWriteNode.SetValue(Variant(true));
        };

        /*************************Operation Mode Functions*************************/
        void automatic()
        {
            automaticNode.SetValue(Variant(true));
        }

        void cycleEnd()
        {
            cycleEndNode.SetValue(Variant(true));
        };

        /*************************Monitor Carrier Arrival Functions*************************/
        bool readArrivingSensor() {
            return arrivingSensorNode.GetValue().As<bool>();
        };

        bool readArrivalSensor() {
            // Logger->debug("Read Arrival Sensor");
            return arrivalSensorNode.GetValue().As<bool>();
        };

        void keepMonitoringCarrier(bool *isStop) {
            Logger->debug("Keep Monitoring Carrier");
            Variant cycleEndVariant(true);

            while (!readArrivingSensor()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            while (!readArrivalSensor()) {
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            // cycleEnd(); 避免套函式延誤時間
            cycleEndNode.SetValue(cycleEndVariant);
            *isStop = true;
        };

        void monitorCarrierArrivalThenStop(bool *isStopp) {
            std::thread monitorCarrierArrivalThread(&MagBack::keepMonitoringCarrier, this, isStopp);
            Logger->debug("Monitoring Carrier Arrival Then Stop");
            monitorCarrierArrivalThread.detach();
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

    };


    class ASRS32
    {
    public:
        ASRS32(std::shared_ptr<spdlog::logger> logger) : Logger(logger)
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

        virtual ~ASRS32() 
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
        void automatic() 
        {
            automaticNode.SetValue(Variant(true));
        }

        void cycleEnd()
        {
            cycleEndNode.SetValue(Variant(true));
        };

        /*************************Monitor Carrier Arrival Functions*************************/
        // Stopper 1
        bool readStopper1ArrivingSensor() {
            return arriving1SensorNode.GetValue().As<bool>();
        };

        bool readStopper1ArrivalSensor() {
            // Logger->debug("Read Arrival Sensor");
            return arrival1SensorNode.GetValue().As<bool>();
        };

        void keepMonitoringCarrierStopper1(bool *isStop) {
            Logger->debug("Keep Monitoring Carrier");
            Variant cycleEndVariant(true);

            while (!readStopper1ArrivingSensor()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            while (!readStopper1ArrivalSensor()) {
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            // cycleEnd(); 避免套函式延誤時間
            cycleEndNode.SetValue(cycleEndVariant);
            *isStop = true;
        };

        void monitorCarrierArrivalThenStopStopper1(bool *isStopp) {
            std::thread monitorCarrierArrivalStooper1Thread(&MagBack::keepMonitoringStopper1Carrier, this, isStopp);
            Logger->debug("Monitoring Carrier Arrival Then Stop");
            monitorCarrierArrivalStopper1Thread.detach();
        };

        // Stopper 2
        bool readStopper2ArrivingSensor() {
            return arriving1SensorNode.GetValue().As<bool>();
        };

        bool readStopper2ArrivalSensor() {
            // Logger->debug("Read Arrival Sensor");
            return arrival1SensorNode.GetValue().As<bool>();
        };

        void keepMonitoringCarrierStopper2(bool *isStop) {
            Logger->debug("Keep Monitoring Carrier");
            Variant cycleEndVariant(true);

            while (!readStopper2ArrivingSensor()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            while (!readStopper2ArrivalSensor()) {
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            // cycleEnd(); 避免套函式延誤時間
            cycleEndNode.SetValue(cycleEndVariant);
            *isStop = true;
        };

        void monitorCarrierArrivalThenStopStopper2(bool *isStopp) {
            std::thread monitorCarrierArrivalStooper1Thread(&MagBack::keepMonitoringStopper2Carrier, this, isStopp);
            Logger->debug("Monitoring Carrier Arrival Then Stop");
            monitorCarrierArrivalStopper2Thread.detach();
        };

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
    };
}