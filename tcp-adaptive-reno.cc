/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>,
 *          Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 *          Greeshma Umapathi
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "tcp-adaptive-reno.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE("TcpAdaptiveReno");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(TcpAdaptiveReno);

TypeId
TcpAdaptiveReno::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TcpAdaptiveReno")
                            .SetParent<TcpWestwoodPlus>()
                            .SetGroupName("Internet")
                            .AddConstructor<TcpAdaptiveReno>();
    return tid;
}

TcpAdaptiveReno::TcpAdaptiveReno()
    : TcpWestwoodPlus(),
      m_rtt_min(0),
      m_rtt_curr(0),
      m_rtt_j(0),
      m_rtt_cong(0),
      m_rtt_cong_prev(0),
      m_w_inc(0),
      m_w_base(0),
      m_w_probe(0)
{
    NS_LOG_FUNCTION(this);
}

TcpAdaptiveReno::TcpAdaptiveReno(const TcpAdaptiveReno& sock)
    : TcpWestwoodPlus(sock),
      m_rtt_min(0),
      m_rtt_curr(0),
      m_rtt_j(0),
      m_rtt_cong(0),
      m_rtt_cong_prev(0),
      m_w_inc(0),
      m_w_base(0),
      m_w_probe(0)
{
    NS_LOG_FUNCTION(this);
}

TcpAdaptiveReno::~TcpAdaptiveReno()
{
}

void
TcpAdaptiveReno::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt)
{
    NS_LOG_FUNCTION(this << tcb << packetsAcked << rtt);

    if (rtt.IsZero())
    {
        NS_LOG_WARN("RTT measured is zero!");
        return;
    }

    m_ackedSegments += packetsAcked;

    if (m_rtt_min.IsZero() || rtt < m_rtt_min)
    {
        m_rtt_min = rtt;
    }

    m_rtt_curr = rtt;

    TcpWestwoodPlus::EstimateBW(rtt, tcb);
}

double
TcpAdaptiveReno::EstimateCongestionLevel()
{
    float m_a = 0.85; // exponential smoothing factor
    if (m_rtt_cong_prev < m_rtt_min)
        m_a = 0; // 1st value of RTT_j when j = 1

    m_rtt_cong = m_a * m_rtt_cong_prev + (1 - m_a) * m_rtt_j;
    double c = std::min((m_rtt_curr.GetSeconds() - m_rtt_min.GetSeconds()) /
                            (m_rtt_cong.GetSeconds() - m_rtt_min.GetSeconds()),
                        1.0);
    return c;
}

void
TcpAdaptiveReno::EstimateIncWnd(Ptr<TcpSocketState> tcb)
{
    double congestion = EstimateCongestionLevel();
    double factor_m = 1000;

    double w_inc_max = static_cast<double>(m_currentBW.Get().GetBitRate()) / factor_m *
                       static_cast<double>(tcb->m_segmentSize * tcb->m_segmentSize);

    double alpha = 10;
    double beta = 2 * w_inc_max * ((1 / alpha) - ((1 / alpha + 1) / (std::exp(alpha))));
    double gamma = 1 - (2 * w_inc_max * ((1 / alpha) - ((1 / alpha + 0.5) / (std::exp(alpha)))));

    m_w_inc = static_cast<int32_t>((w_inc_max / std::exp(alpha * congestion)) + (beta * congestion) + gamma);
}

void
TcpAdaptiveReno::CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
    NS_LOG_FUNCTION(this << tcb << segmentsAcked);

    if (segmentsAcked > 0)
    {
        EstimateIncWnd(tcb);

        // base_window: new reno
        uint32_t seg_size = tcb->m_segmentSize;

        double t = seg_size * seg_size * 1.0 / tcb->m_cWnd.Get();
        m_w_base = m_w_base + static_cast<uint32_t> (std::max(1.0, t));

        // probe window
        m_w_probe =
            static_cast<int32_t>(std::max(m_w_probe + m_w_inc * 1.0 / tcb->m_cWnd.Get(), 0.0));

        tcb->m_cWnd = m_w_base + m_w_probe;
    }
}

uint32_t
TcpAdaptiveReno::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight)
{
    m_rtt_cong_prev = m_rtt_cong;
    m_rtt_j = m_rtt_curr;

    double congestion = EstimateCongestionLevel();

    m_w_base = std::max(2 * tcb->m_segmentSize, static_cast<uint32_t>(tcb->m_cWnd / (1.0 + congestion)));
    m_w_probe = 0;

    return m_w_base;
}

Ptr<TcpCongestionOps>
TcpAdaptiveReno::Fork()
{
    return CreateObject<TcpAdaptiveReno>(*this);
}

} // namespace ns3
