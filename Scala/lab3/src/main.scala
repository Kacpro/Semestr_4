

object Appl 
{
  def readFile(fileName: String) = try 
  {
     println("Opening the file...")
     val inFile = scala.io.Source.fromFile("logins.txt")
     try
     {
       for (line <- inFile.getLines) println(line)
       val x = 100 / inFile.getLines.length
     }
     finally
     {
       println("Closing the file")
       inFile.close
     }
   } 
   catch 
   {
     case ex: java.io.FileNotFoundException => println(ex.getMessage)
     case ex: Throwable => println("Default exception handler: " + ex.getMessage)
   }

   def main(args: Array[String])
   {
     readFile("logins.txt")
   }
}



package p1
{
  package p2
  {
    package p3
    {
      class A3
    }
  }
}


































