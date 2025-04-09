# Taller1-Arqui2
Para la ejecución de este programa primero lo que hay que hacer es correr el Makefile con el comando `make`
una vez ejecutado el Makefile este generara el ejecutable, si solo lo corres en la terminal como `./tls_mod`el usara por defecto 2 variable compartidas y 4 threads
en el caso de que quieras especificar el número de Threas es solo de ejecutar lo siguiente `./tls_mod <Numero_threads> <Numero_variables>`
Para hacer las pruebas automatizadas se crea un archivo llamado test.py que realiza una ejecución haciendo un barrido de 1 a 11 thread y de 1 a 2 variables compartidas para graficarlas.
