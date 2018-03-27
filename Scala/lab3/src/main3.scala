

import p1.p2.C2._
object Appl
{
  import p1._
  new C1
  import p2.{ C21, C22 => MyC22, C23 => _ }
  new C21
  new MyC22
  def main(args: Array[String])
  {
    m10C2()
    m20C2(new C1)
  }
}
