install: Calendar.cpp
	g++ Calendar.cpp -lncurses -o /usr/local/bin/Calendar
clean: Calendar
	rm -f /usr/local/bin/Calendar
