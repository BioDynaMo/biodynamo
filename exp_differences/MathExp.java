
class MathExp {
  public static void main(String[] args) {
    double x = -0.0028724514195400627;
    double result = Math.exp(x);
    System.out.println(result);
    System.out.println(doubleToHexStr(result));
  }

  static String doubleToHexStr(double d) {
    long binary = Double.doubleToLongBits(d);
    return Long.toHexString(binary);
  }
}
