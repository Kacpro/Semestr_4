package p1
{
 class A1T { new A1B }
   package p2
   {
     class A2T { new A2B; new A1T; new A1B }
     package p3
     {
       class A3 { new A2T; new A1T }
     }
     class A2B {}
   } 
   class A1B {}
} 


object Appl
{
  def main(args: Array[String])
  {
    new p1.A1T
    new p1.A1B
    new p1.p2.A2T
    new p1.p2.A2B
    new p1.p2.p3.A3
  }
}

