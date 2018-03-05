class Person(val givenName : String, var surname : String, val id : String) {
	def name = givenName + " " + surname
}

object Appl extends App {
	val p = new Person("Jan", "Kowalski", "1234567890")
	println(p.name)
}