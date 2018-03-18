class Int2DVec private (val x: Int, val y: Int) {
	def +(other: Int2DVec) = new Int2DVec(x+other.x, y+other.y)
	def unary_- = new Int2DVec(-x, -y)
	
	def -(other: Int2DVec) = new Int2DVec(x-other.x, y-other.y)
	def *(other: Int2DVec) = x*other.x + y*other.y
	
	override def toString() = "(" + x.toString + "," + y.toString + ")"
}

import scala.math

object Int2DVec {
	def apply (x : Int, y : Int) = new Int2DVec(x, y)
	def apply() = new Int2DVec(0, 0)
	def apply(prototype : Int2DVec) = new Int2DVec(prototype.x, prototype.y)
	
//	def unitVectorOf( vector : Int2DVec ) = new Int2DVec((vector.x / math.pow(vector.x * vector.x + vector.y * vector.y, 0.5)) : Int, (vector.y / math.pow(vector.x * vector.x + vector.y * vector.y, 0.5)) : Int)
}


object Appl {
	def main(args: Array[String]) {
		val v1 = Int2DVec(1, 2)
		val v2 = Int2DVec(10, 20)
		val v3 = v1 + v2
		val v4 = -v2
		println(v3)
		println(v4)
		
		println(Int2DVec())
		println(Int2DVec(v1 + v1 + v2))
	}
}