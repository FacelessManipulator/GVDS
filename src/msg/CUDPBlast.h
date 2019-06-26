#pragma once
#include <udt.h>
#include <ccc.h>
class CUDPBlast: public CCC
{
public:
   CUDPBlast()
   {
      m_dPktSndPeriod = 10; 
      m_dCWndSize = 83333.0;
   }

public:
   void setRate(double mbps)
   {
      m_dPktSndPeriod = (m_iMSS * 8.0) / mbps;
   }
};
