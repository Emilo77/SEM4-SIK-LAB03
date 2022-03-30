# Scenariusz 3 - UDP
1 Wprowadzenie

Gniazda (Berkeley sockets) to najpopularniejsze API do komunikacji sieciowej po protokołach TCP i UDP.

Protokół UDP jest: bezpołączeniowy, datagramowy i nie zapewnia niezawodności.
1.1 Architektura klient-serwer

Komunikacja w gniazdach odbywa się między dwoma stronami o różnych rolach: klienta i serwera.

Serwer musi wybrać port, czyli 16-bitowy numer, który będzie identyfikować proces serwera w systemie operacyjnym. Popularne usługi mają ogólnie znane numery portów np. DNS - 53. Serwer może także wybrać interfejsy sieciowe, na których będzie nasłuchiwał.

Klient łączy się z wybranym serwerem podając jego adres IP i numer portu.
2. Serwer echo

Serwer echo czyta otrzymany od klienta ciąg znaków, a następnie odsyła go do z powrotem do klienta.
2.1 Przepływ sterowania - sekwencja wywołań funkcji API

socket -> bind ->  (recvfrom/sendto) -> [ close ]

2.2 socket: utworzenie gniazda komunikacji sieciowej

int socket(int domain, int type, int protocol);

    domain: rodzina protokołów, które będą używane do komunikacji (np. AF_INET - IPv4; AF_INET6 - IPv6; AF_UNIX)

    type: rodzaj połączenia np. SOCK_STREAM (połączeniowe, np. TCP), SOCK_DGRAM (datagramowe, np. UDP), SOCK_RAW

    protocol: wybór protokołu; 0, gdy możliwy jest tylko jeden protokół dla ustalonych domain i type

Funkcja przekazuje w wyniku deskryptor gniazda lub -1 w przypadku błędu.

Gniazda otwierane przez socket() muszą być zamykane przez close(). Inaczej zużywają zasoby systemowe.
2.3 bind: związanie lokalnego adresu z gniazdem

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    sockfd: deskryptor gniazda, z którym związujemy adres

    addr: wskaźnik do struktury opisującej adres (interfejs i port)

    addrlen: długość struktury opisującej adres

Struktura opisująca adres jest typu struct sockaddr (<sys/socket.h>) i reprezentuje ogólny typ adresu:

struct sockaddr {
  uint8_t     sa_len; /* To pole nie zawsze występuje. */
  sa_family_t sa_family;
  char        sa_data[14]
};

Przy otwieraniu gniazda protokołu UDP/IPv4 używa się struct sockaddr_in (<netinet/in.h>):

struct sockaddr_in {
     uint8_t        sin_len; /* To pole nie zawsze występuje. */
     sa_family_t    sin_family;
     in_port_t      sin_port;
     struct in_addr sin_addr;
     char           sin_zero[8];
};

struct sockaddr_in jest kompatybilna ze struct sockaddr: pierwsze dwa pola mają tę samą długość, a sin_port i sin_addr wypełnia fragment sa_data. Jest to analogiczne do dziedziczenia w programowaniu obiektowym.

Zawartość pól sin_port oraz sin_addr wpisuje się w sieciowej kolejności bajtów (big endian). Jeżeli chcemy napisać program, który może potencjalnie komunikować się z architekturami o różnych kolejnościach bajtów, to także powinniśmy zadbać o stosowanie sieciowego porządku bajtów i odpowiednie konwersje. W praktyce dobrze jest założyć, że zawsze w danych przesyłanych przez sieć stosujemy sieciowy porządek bajtów dla danych binarnych.

Do konwersji liczb między kolejnością hosta a sieciową służą funkcje:

    #include <arpa/inet.h>

    uint32_t htonl(uint32_t hostlong);
    uint16_t htons(uint16_t hostshort);
    uint32_t ntohl(uint32_t netlong);
    uint16_t ntohs(uint16_t netshort);

    #include <endian.h>

    uint16_t htobe16(uint16_t host_16bits);
    uint16_t htole16(uint16_t host_16bits);
    uint16_t be16toh(uint16_t big_endian_16bits);
    uint16_t le16toh(uint16_t little_endian_16bits);

    uint32_t htobe32(uint32_t host_32bits);
    uint32_t htole32(uint32_t host_32bits);
    uint32_t be32toh(uint32_t big_endian_32bits);
    uint32_t le32toh(uint32_t little_endian_32bits);

    uint64_t htobe64(uint64_t host_64bits);
    uint64_t htole64(uint64_t host_64bits);
    uint64_t be64toh(uint64_t big_endian_64bits);
    uint64_t le64toh(uint64_t little_endian_64bits);

2.4 Obsługa klienta: recvfrom / sendto

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                     struct sockaddr *src_addr, socklen_t *addrlen);

    sockfd: gniazdo, które zostało utworzone za pomocą socket()
    buf: miejsce na dane do odebrania
    len: długość bufora na odbierane dane
    flags: znaczniki wskazujące jak odbierać dane
    src_addr: miejsce na adres nadającego; gdy NULL, adres nie jest podawany

    addrlen: długość pola adresu; może być NULL, gdy src_addr jest NULL

    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen);

    sockfd: j.w.
    buf: miejsce z danymi do wysłania
    len: liczba oktetów do wysłania
    flags: znaczniki wskazujące, jak wysyłać dane
    dest_addr: adres odbiorcy danych

    addrlen: długość pola adresu

2.5 close: zamknięcie gniazda

int close(int fd);

W przypadku UDP działanie close() ogranicza się do zwolnienia zasobów.
2.6 Ćwiczenia

    Uruchom serwer ( ./echo-server ).

    Znajdź gniazdo serwera w wykazie otwartych gniazd (netstat -lunp).

    Podłącz się do serwera programem netcat (nc -u łączy po UDP).

    Podłącz się do serwera koleżanki lub kolegi.

    Podłącz się do jednego serwera na raz dwoma klientami. Czy serwer otrzymuje dane od obu klientów?

    Zmniejsz rozmiar bufora odbieranych danych do 10 bajtów. Spróbuj wysłać dłuższe komunikaty. Czy serwer jest w stanie odczytać dane poza 10 bajtem?

3 Klient echo
3.1 Przepływ sterowania - sekwencja wywołań funkcji API

socket -> (recvfrom/sendto)-> [ close ]

3.2 socket: stworzenie gniazda komunikacji sieciowej

Jak u serwera.
3.3 recvfrom/sendto: wysyłanie i obieranie pakietów

Jak u serwera. Klient nie musi związywać z gniazdem numeru portu. Przed wysłaniem pierwszego komunikatu port zostanie przydzielony przez system operacyjny.
3.4 close: zamknięcie gniazda

Jak u serwera.
3.5 getaddrinfo: tłumaczenie adresów na struct sockaddr

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);

getaddrinfo() tworzy adres internetowy (res) na podstawie: nazwy hosta (node), nazwy usługi (service) i podpowiedzi (hints).

    node może być adresem IPv4 (np. 8.8.8.8), IPv6 albo nazwą dziedzinową (np. localhost, green01, students.mimuw.edu.pl)

    service może być numerem portu (np. 10001) albo nazwą usługi (np. http)

    Zawartość struktury addrinfo:

    struct addrinfo {
            int              ai_flags;     // dodatkowe opcje
            int              ai_family;    // rodzina adresów: AF_INET (IPv4),
                                           // AF_INET6 (IPv6),
                                           // AF_UNSPEC (pozwalamy na wybór)
            int              ai_socktype;  // typ: np. SOCK_DGRAM, patrz: socket()
            int              ai_protocol;  // protokół, patrz: socket()
            size_t           ai_addrlen;   // długość adresu
            struct sockaddr *ai_addr;      // adres w postaci struct sockaddr
            char            *ai_canonname; // nazwa hosta
            struct addrinfo *ai_next;      // wskaźnik na następną odpowiedź
    };

Uwaga: pamięć na res jest alokowana przez bibliotekę, więc musi być zwolniona przez freeaddrinfo(struct addrinfo* res).
3.6 Alternatywny przepływ sterowania

socket -> connect -> (recv/send)-> [ close ]

3.7 connect: dla UDP - związanie z gniazdem zdalnego adresu

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    sockfd: deskryptor gniazda

    addr: wskaźnik do struktury opisującej adres, z którym będziemy się łączyć, jak dla bind()

    addrlen: długość struktury opisującej adres

W przypadku połączeń po protokole UDP:

    addr ustala domyślny adres do wysyłania przez sendto/send() oraz ogranicza zakres adresów, z których będziemy odbierać dane do addr

    można wykonywać connect() wiele razy na tym samym gnieździe

    wykonanie tego wywołania jest opcjonalne

3.8 recv/send: odbieranie i wysyłanie po ustaleniu adresu zdalnego

ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);

ssize_t read(int sockfd, void *buf, size_t len);
ssize_t write(int sockfd, const void *buf, size_t len);

Jak dla funkcji recvfrom() i sendto().
3.9 Ćwiczenia

    Uruchom klienta i połącz się ze swoim lokalnym serwerem.

    Połącz się z serwerem sąsiada.

    Uruchom wireshark. Następnie uruchom klienta. Zaobserwuj pakiety wysyłane przez klienta i odsyłane przez serwer.

    Czy jesteś w stanie wysyłać dane długości większej niż MTU sieci, w której działasz?

    Czy jesteś w stanie wysyłać dane długości większej niż 65536 bajtów? Zbadaj, ile dokładnie wynosi maksymalny rozmiar danych, które można przesłać w Twojej sieci.

    Zmodyfikuj klienta tak, aby korzystał z funkcji connect(), send() i recv().

    Po wywołaniu connect() zawieś wykonanie klienta (np. przez sleep(30)). Zaobserwuj, czy connect() powoduje wysłanie jakichś pakietów, czy też wysyłane one są dopiero po pierwszym send().

    W podobny sposób opóźnij wykonanie close(). Czy w rezultacie wykonania close() klient lub serwer wysyłają jakieś pakiety?

4 Ćwiczenie punktowane (1 pkt)

Zmodyfikuj klienta w ten sposób, aby przyjmował dwa dodatkowe parametry: liczbę pakietów do wysłania (n) i rozmiar porcji danych (k). Klient n razy przesyła k dowolnych bajtów do serwera. Do skonstruowania takiego komunikatu możesz użyć funkcji memset().

Zmodyfikuj serwer w ten sposób, aby czytał w pętli komunikaty od klienta. Po otrzymaniu danych serwer dopisuje je do pliku o ustalonej nazwie, a na standardowe wyjście podaje jedynie liczbę otrzymanych bajtów.

Przetestuj różne wartości k (np. 10, 100, 1000, 5000). Zaobserwuj, czy wartości wypisywane przez serwer zależą od parametru klienta. Uruchom klienta i serwera na dwóch różnych maszynach.

Rozwiązania można prezentować w trakcie zajęć nr 3 lub 4.
