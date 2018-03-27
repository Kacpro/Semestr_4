

package p1
{
  class C1
  {
    println("C1")
    def m1C1() { println("m1C1()") }
  }
  package p2
  {
    class C21 { println("C21") }
    class C22 { println("C22") }
    class C23 { println("C23") }
    object C2
    {
      def m10C2() { println("m10C2") }
      def m20C2(c1: C1)
      {
        import c1._
        println("m20C2")
        m1C1()
      }
    }
  }
}

