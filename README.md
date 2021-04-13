# Process-Synchronization-with-C-Programming-in-LINUX
POSIX semaphores are used for synchronization and shared memory is used for information sharing between processes. The search tree I created was used.

Processleri senkronize etmek icin POSIX semaphores kullanılacaktır.
Ana thread tarafından N adet isci thread olusturacak bir program kodlayınız. Program K adet dugum sayısına sahip paylasılabilir bir arama agacı icermelidir. Agac, girdi dosyaları icerisindeki su ana kadar olan en buyuk K degerlerini tutacaktır.Input dosyalarını islemek icin WORKER PROCESS ler kullanmalı.Input dosyası icerisinden degerler okunduktan sonra, WORKER PROCESS ler eger ¸sartlar saglanıyorsa degeri agaca ekleyecektir degilse integer deger bırakılacak ve dosyadaki diger integer deger alınacak. Parent ve child processler arasında bilgi paylasımı icin shared memory kullanacaksınız. Input dosyaları icerisindeki su ana kadar olan en buyuk K degerlerini tutacaktır. Yani input dosyasının child process tarafından islenirken dinamik olarak degismesi gerekir. Integer degerleri shared
memory de saklanmalıdır(her bir child process i¸cin 1 dizi gibi.).
Programın çıktısı: part2 K N infile1 ... infileN outfile şeklinde olmalıdır.
