CXX = g++
#CFLAGS = -std=c99 -Wall -I.
CXXFLAGS = -g -O3 -Wall -Wunused-variable
CXXINCLUDE = -I/usr/local/include

LINKER = g++ -o
LFLAGS = -Wall -lm
LLIB = -L/usr/local/lib -lwiringPi -lwiringPiDev -lpthread

SRCDIR = src
OBJDIR = obj
BINDIR = bin
rm = rm -f

#ad5933.o: ad5933.c ad5933.h
#	@echo "Compilando "$<".."
#	${CXX} ${CXXFLAGS} -c ${SRCDIR}/ad5933.c
#	@echo "Compilando "$<" finalizado."

all: clean $(BINDIR)/pmodia.test $(BINDIR)/pmodia.test.2
	@echo "Proceso Finalizado."

$(BINDIR)/pmodia.test:
	@echo "Compilando "$@" - Iniciando"
	$(CXX) $(CXXFLAGS) $(CXXINCLUDE) -pipe $(LFLAGS) $(LLIB) -o $(BINDIR)/pmodia.test $(SRCDIR)/test.c
	@echo "Compilando "$@" - Finalizado"

$(BINDIR)/pmodia.test.2:
	@echo "Compilando "$@" - Iniciando"
	$(CXX) $(CXXFLAGS) $(CXXINCLUDE) -pipe $(LFLAGS) $(LLIB) -o $(BINDIR)/pmodia.test.2 $(SRCDIR)/test2.c
	@echo "Compilando "$@" - Finalizado"

clean:
	@echo "Limpiando archivos ejecutables/objeto"
	rm -f $(BINDIR)/pmodia.test
	rm -f $(BINDIR)/pmodia.test.2
	@echo "Ejecutables removido."


