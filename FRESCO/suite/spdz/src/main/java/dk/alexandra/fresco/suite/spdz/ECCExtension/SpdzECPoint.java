package dk.alexandra.fresco.suite.spdz.ECCExtension;

import iaik.security.ec.math.curve.ECPoint;
import iaik.security.ec.math.curve.EllipticCurve;
import java.io.Serializable;
import java.math.BigInteger;

import dk.alexandra.fresco.suite.spdz.ECCExtension.SECPoint;

/**
 * A Spdz class for ECC Points
 *
 * @author ******
 */
public class SpdzECPoint implements SECPoint, Serializable {

  private static final long serialVersionUID = 5882876872861854360L;
  private final ECPoint share;
  private final ECPoint mac;

  public SpdzECPoint(ECPoint share, ECPoint mac) {
    this.share = share;
    this.mac = mac;
  }

  public static SpdzECPoint multiplyPoint(ECPoint p, BigInteger share, BigInteger mac) {

    ECPoint ECshare = p.clone().multiplyPoint(share);
    ECPoint ECmac = p.clone().multiplyPoint(mac);
    return new SpdzECPoint(ECshare, ECmac);
  }

  @Override
  public String toString() {
    return "spdz((" +
      share.getCoordinate().getX() + ", " + share.getCoordinate().getY()
      + "), (" +
      mac.getCoordinate().getX() + ", " + mac.getCoordinate().getY()
      + "))";
  }

  @Override
  public SECPoint out() {
    return this;
  }

  public ECPoint getShare() {
    return share;
  }

  public ECPoint getMac() {
    return mac;
  }

  public byte[] serializeShare() {
    return share.encodePoint();
  }

  public EllipticCurve getCurve() {
    return share.getCurve();
  }
}
