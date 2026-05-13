# Derleyici
CC = gcc

# Başlık dosyalarının (.h) aranacağı dizin
CFLAGS = -I ./include/

# Tüm adımları sırasıyla çalıştırır
hepsi: derle bagla

# src klasöründeki .c dosyalarını derleyip lib klasörüne .o olarak atar
derle:
	$(CC) $(CFLAGS) -c ./src/arsiv.c -o ./lib/arsiv.o
	$(CC) $(CFLAGS) -c ./src/cikarma.c -o ./lib/cikarma.o
	$(CC) $(CFLAGS) -c ./src/main.c -o ./lib/main.o

# lib klasöründeki .o dosyalarını bağlayarak bin klasörüne çalıştırılabilir dosyayı atar
bagla:
	$(CC) ./lib/main.o ./lib/arsiv.o ./lib/cikarma.o -o ./bin/tarsau

# Oluşan gereksiz .o dosyalarını ve programı silmek için (make temizle)
temizle:
	rm -f ./lib/*.o ./bin/tarsau