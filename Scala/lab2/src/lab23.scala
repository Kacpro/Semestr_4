
trait Fraction {
  val num: Int
  val denom: Int
  def *(other: Fraction): Fraction
}
  
trait Loggable {
  def log(timeStamp: Long, msg: String) = println("[" + timeStamp.toString + "]: " + msg)
}
  
object Fraction {
  // one of the "creational patterns/idioms"
  def apply(num: Int, denom: Int, loggable: Boolean = false): Fraction =
    if (loggable) new LoggableImpl(num, denom) else new DefaultImpl(num, denom) 
  
  private class DefaultImpl(val num: Int, val denom: Int) extends Fraction {
    override def *(other: Fraction): Fraction =
      Fraction(this.num * other.num, this.denom * other.denom)
    override def toString() = num.toString + "/" + denom.toString
  }
   
  private class LoggableImpl(num: Int, denom: Int) extends DefaultImpl(num, denom) with Loggable {
    def timeStamp = System.nanoTime // to keep the example short...
    override def *(other: Fraction): Fraction = {
      log(timeStamp, "multiplying " + this.toString + " by " + other.toString)
      Fraction(this.num * other.num, this.denom * other.denom, true) // super.*(other) is not loggable
    }
  }
}
  
object Appl {
  def main(agrs: Array[String]) {
    val f1 = Fraction(3, 7)
    val f2 = Fraction(4, 9)
    val f3 = Fraction(1, 19, true)
    val f1f2 = f1 * f2
    println(f1.toString + " * " + f2.toString + " = " + f1f2)
    println(f3.toString + " * " + f2.toString + " * " + f1.toString + " = " + f3 * f2 * f1)
  }
}