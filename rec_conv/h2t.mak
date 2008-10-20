html2txt: html2txt.c dfconv.c
	gcc -Wall -O2 -o $@ $^

clean:
	rm html2txt.exe
