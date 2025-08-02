#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/object-ptr-container.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"

// Default Network Topology
//
//  n2                    n4
//  |                     |
//  |-----n0-------n1-----|
//  |                     |
//  n3                    n5

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("1905024");

/* My App ------------------------------------------------ */
class MyApp : public Application
{
  public:
    MyApp();
    virtual ~MyApp();

    /**
     * Register this type.
     * \return The TypeId.
     */
    static TypeId GetTypeId(void);
    void Setup(Ptr<Socket> socket,
               Address address,
               uint32_t packetSize,
               DataRate dataRate,
               uint32_t simultime);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void ScheduleTx(void);
    void SendPacket(void);

    Ptr<Socket> m_socket;
    Address m_peer;
    uint32_t m_packetSize;
    DataRate m_dataRate;
    EventId m_sendEvent;
    bool m_running;
    uint32_t m_packetsSent;
    uint32_t m_simultime;
};

MyApp::MyApp()
    : m_socket(0),
      m_peer(),
      m_packetSize(0),
      m_dataRate(0),
      m_sendEvent(),
      m_running(false),
      m_packetsSent(0),
      m_simultime(0)
{
}

MyApp::~MyApp()
{
    m_socket = 0;
}

/* static */
TypeId
MyApp::GetTypeId(void)
{
    static TypeId tid =
        TypeId("MyApp").SetParent<Application>().SetGroupName("Tutorial").AddConstructor<MyApp>();
    return tid;
}

void
MyApp::Setup(Ptr<Socket> socket,
             Address address,
             uint32_t packetSize,
             DataRate dataRate,
             uint32_t simultime)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_dataRate = dataRate;
    m_simultime = simultime;
}

void
MyApp::StartApplication(void)
{
    m_running = true;
    m_packetsSent = 0;
    if (InetSocketAddress::IsMatchingType(m_peer))
    {
        m_socket->Bind();
    }
    else
    {
        m_socket->Bind6();
    }
    m_socket->Connect(m_peer);
    SendPacket();
}

void
MyApp::StopApplication(void)
{
    m_running = false;

    if (m_sendEvent.IsRunning())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close();
    }
}

void
MyApp::SendPacket(void)
{
    Ptr<Packet> packet = Create<Packet>(m_packetSize);
    m_socket->Send(packet);

    if (Simulator::Now().GetSeconds() < m_simultime)
        ScheduleTx();
}

void
MyApp::ScheduleTx(void)
{
    if (m_running)
    {
        Time tNext(Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule(tNext, &MyApp::SendPacket, this);
    }
}

/* -------------------------------------------------------------------------------------*/

static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    *stream->GetStream() << Simulator::Now().GetSeconds() << " " << newCwnd << std::endl;
}

int
main(int argc, char* argv[])
{
    // given value
    int n_leafNode = 2;
    std::string leafDataRate = std::string("1Gbps");
    std::string leafDelay = std::string("1ms");
    int bottleneckDelay = 100;
    uint32_t payloadSize = 1024;
    int simulationTime = 30;

    // variable value
    std::string filename = std::string("test");
    std::string parameter = std::string("datarate");
    std::string tcpAlgo1 = std::string("TcpNewReno");
    std::string tcpAlgo2 = std::string("TcpWestwoodPlus");
    int bottleneckRate = 50;
    double errorRateExp = 6;

    CommandLine cmd(__FILE__);
    cmd.AddValue("bottleneckRate", "Data rate of bottleneck devices in Mbps", bottleneckRate);
    cmd.AddValue("errorRateExp", "Error rate exponent of bottleneck devices", errorRateExp);
    cmd.AddValue("tcpAlgo1", "TCP algorithm used in first flow", tcpAlgo1);
    cmd.AddValue("tcpAlgo2", "TCP algorithm used in second flow", tcpAlgo2);
    cmd.AddValue("filename", "Output file name", filename);
    cmd.AddValue("filename", "Output file name", filename);
    cmd.Parse(argc, argv);

    /* Create and initialize nodes ---------------------------------------------------*/
    PointToPointHelper leftHelper, rightHelper, bottleneckHelper;

    // set data rate and delay
    bottleneckHelper.SetDeviceAttribute("DataRate",
                                        StringValue(std::to_string(bottleneckRate) + "Mbps"));
    bottleneckHelper.SetChannelAttribute("Delay",
                                         StringValue(std::to_string(bottleneckDelay) + "ms"));
    leftHelper.SetDeviceAttribute("DataRate", StringValue(leafDataRate));
    leftHelper.SetChannelAttribute("Delay", StringValue(leafDelay));
    rightHelper.SetDeviceAttribute("DataRate", StringValue(leafDataRate));
    rightHelper.SetChannelAttribute("Delay", StringValue(leafDelay));

    // set queue
    leftHelper.SetQueue(
        "ns3::DropTailQueue",
        "MaxSize",
        StringValue(std::to_string((bottleneckDelay * bottleneckRate * 1000) / payloadSize) + "p"));
    rightHelper.SetQueue(
        "ns3::DropTailQueue",
        "MaxSize",
        StringValue(std::to_string((bottleneckDelay * bottleneckRate)) + "p"));

    // set error model
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(std::pow(10, -1 * errorRateExp)));
    // p2pdHelper.GetRight()->SetAttribute("ReceiveErrorModel", PointerValue(em));
    bottleneckHelper.SetDeviceAttribute("ReceiveErrorModel", PointerValue(em));

    PointToPointDumbbellHelper p2pdHelper(n_leafNode,
                                          leftHelper,
                                          n_leafNode,
                                          rightHelper,
                                          bottleneckHelper);

    /* Internet stack and addressing ---------------------------------------------------*/
    InternetStackHelper stack1, stack2;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                       StringValue(std::string("ns3::") + tcpAlgo1));
    stack1.Install(p2pdHelper.GetLeft(0));
    stack1.Install(p2pdHelper.GetRight(0));
    stack1.Install(p2pdHelper.GetLeft());
    stack1.Install(p2pdHelper.GetRight());

    Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                       StringValue(std::string("ns3::") + tcpAlgo2));
    stack2.Install(p2pdHelper.GetLeft(1));
    stack2.Install(p2pdHelper.GetRight(1));

    Ipv4AddressHelper leftAddr("10.1.1.0", "255.255.255.0"), rightAddr("10.2.1.0", "255.255.255.0"),
        bttlenkAddr("10.3.1.0", "255.255.255.0");
    p2pdHelper.AssignIpv4Addresses(leftAddr, rightAddr, bttlenkAddr);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    /* App install ---------------------------------------------------*/
    uint16_t sinkPort = 8080;
    for (int i = 0; i < n_leafNode; i++)
    {
        // Sink app
        Address sinkAddress(InetSocketAddress(p2pdHelper.GetRightIpv4Address(i), sinkPort));
        PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), sinkPort++));
        ApplicationContainer sinkApps = packetSinkHelper.Install(p2pdHelper.GetRight(i));
        sinkApps.Start(Seconds(0.5));
        sinkApps.Stop(Seconds(simulationTime + 0.5));

        // My app
        Ptr<Socket> ns3TcpSocket =
            Socket::CreateSocket(p2pdHelper.GetLeft(i), TcpSocketFactory::GetTypeId());
        Ptr<MyApp> app = CreateObject<MyApp>();
        app->Setup(ns3TcpSocket, sinkAddress, payloadSize, DataRate(leafDataRate), simulationTime);
        p2pdHelper.GetLeft(i)->AddApplication(app);
        app->SetStartTime(Seconds(1));
        app->SetStopTime(Seconds(simulationTime));

        // congestion window
        if (filename == "congestion")
        {
            std::ostringstream oss;
            oss << "1905024/" + tcpAlgo1 + "_" + tcpAlgo2 + "_" + filename + "_" << i + 1 << ".dat";
            AsciiTraceHelper asciiTraceHelper;
            Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream(oss.str());
            ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow",
                                                     MakeBoundCallback(&CwndChange, stream));
        }
        sinkPort++;
    }

    /* Tracing and Simulation ---------------------------------------------------*/
    NodeContainer nodes1(p2pdHelper.GetLeft(0), p2pdHelper.GetRight(0));
    NodeContainer nodes2(p2pdHelper.GetLeft(1), p2pdHelper.GetRight(1));
    FlowMonitorHelper flowHelper1, flowHelper2;
    Ptr<FlowMonitor> flowMonitor1 = flowHelper1.Install(nodes1);
    Ptr<FlowMonitor> flowMonitor2 = flowHelper2.Install(nodes2);

    Simulator::Stop(Seconds(simulationTime));

    // AnimationInterface anim("1905024/animation_1.xml");
    // anim.SetMaxPktsPerTraceFile(5000000);

    Simulator::Run();
    Simulator::Destroy();

    /* Calculation ---------------------------------------------------*/
    FlowMonitor::FlowStatsContainer stats1 = flowMonitor1->GetFlowStats();
    FlowMonitor::FlowStatsContainer stats2 = flowMonitor2->GetFlowStats();

    double thrpt1 = 0, thrpt2 = 0;
    for (auto itr : stats1)
        thrpt1 += itr.second.rxBytes * 8 / (simulationTime * 1000);
    for (auto itr : stats2)
        thrpt2 += itr.second.rxBytes * 8 / (simulationTime * 1000);

    /* Bonus ---------------------------------------------------*/
    double numerator = (thrpt1 + thrpt2) * (thrpt1 + thrpt2);
    double denominator = thrpt1 * thrpt1 + thrpt2 * thrpt2;
    double jain_index = numerator / (denominator * 2.0 * n_leafNode);

    if (filename == "congestion")
        return 0;

    std::ofstream ofs1("1905024/" + tcpAlgo1 + "_" + tcpAlgo2 + "_" + filename + "_1.dat",
                       std::ios::out | std::ios::app);
    std::ofstream ofs2("1905024/" + tcpAlgo1 + "_" + tcpAlgo2 + "_" + filename + "_2.dat",
                       std::ios::out | std::ios::app);

    if (filename == "datarate")
    {
        ofs1 << bottleneckRate << " " << thrpt1 << std::endl;
        ofs2 << bottleneckRate << " " << thrpt2 << std::endl;
    }
    else if (filename == "errorRate")
    {
        ofs1 << -1 * errorRateExp << " " << thrpt1 << std::endl;
        ofs2 << -1 * errorRateExp << " " << thrpt2 << std::endl;
    }
    else if (filename == "jainIndexDatarate")
    {
        ofs1 << bottleneckRate << " " << jain_index << std::endl;
    }
    else if (filename == "jainIndexErrorRate")
    {
        ofs1 << -1 * errorRateExp << " " << jain_index << std::endl;
    }

    ofs1.close();
    ofs2.close();

    return 0;
}
