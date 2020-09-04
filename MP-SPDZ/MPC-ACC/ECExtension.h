#pragma once

#include "Protocols/ShamirShare.h"
#include "Protocols/Share.h"
#include "Protocols/Rep3Share.h"
#include "RCurve.h"

using osuCrypto::REccPoint;
using osuCrypto::REccPairingPoint;

// SemiShare
SemiShare<REccPoint> mul(const osuCrypto::REccPoint& lhs, SemiShare<REccPoint::Scalar>& rhs)
{
  SemiShare<REccPoint> r = lhs * rhs;
  return r;
}

// SemiShare
SemiShare<REccPairingPoint> mul(const osuCrypto::REccPairingPoint& lhs, SemiShare<REccPairingPoint::Scalar>& rhs)
{
  SemiShare<REccPairingPoint> r = lhs * rhs;
  return r;
}

// mascot
Share<REccPoint> mul(const osuCrypto::REccPoint& lhs, Share<REccPoint::Scalar>& rhs)
{
  Share<REccPoint> r;
  r.set_share(lhs * rhs.get_share());
  r.set_mac(lhs * rhs.get_mac());
  return r;
}

// mascot
Share<REccPairingPoint> mul(const osuCrypto::REccPairingPoint& lhs, Share<REccPairingPoint::Scalar>& rhs)
{
  Share<REccPairingPoint> r;
  r.set_share(lhs * rhs.get_share());
  r.set_mac(lhs * rhs.get_mac());
  return r;
}

// ShamirShare
ShamirShare<REccPoint> mul(const osuCrypto::REccPoint& lhs, ShamirShare<REccPoint::Scalar>& rhs)
{
  ShamirShare<REccPoint> r = lhs * rhs;
  return r;
}

// ShamirShare
ShamirShare<REccPairingPoint> mul(const osuCrypto::REccPairingPoint& lhs, ShamirShare<REccPairingPoint::Scalar>& rhs)
{
  ShamirShare<REccPairingPoint> r = lhs * rhs;
  return r;
}

// Rep3Share
Rep3Share<REccPoint> mul(const osuCrypto::REccPoint& lhs, Rep3Share<REccPoint::Scalar>& rhs)
{
  Rep3Share<REccPoint> res;
  for (int i = 0; i < res.length; i++)
    res[i] = lhs* rhs[i];
  return res;
}

Rep3Share<REccPairingPoint> mul(const osuCrypto::REccPairingPoint& lhs, Rep3Share<REccPairingPoint::Scalar>& rhs)
{
  Rep3Share<REccPairingPoint> res;
  for (int i = 0; i < res.length; i++)
    res[i] = lhs* rhs[i];
  return res;
}
