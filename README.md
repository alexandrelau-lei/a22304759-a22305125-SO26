# OSSIM — Simulador de Escalonamento

Sistemas Operativos 2026/2027 — Universidade Lusófona.

Simulador de um sistema operativo simplificado. Dois programas comunicam por
*socket* UNIX:

- **`application`** — processo de utilizador. Lê um perfil de *bursts* de um
  ficheiro `.csv` e envia pedidos `RUN` / `BLOCK` ao simulador.
- **`ossim`** — o "SO": servidor de *sockets*, filas de processos e escalonador.

O enunciado deste trabalho está em **[`enunciado.md`](enunciado.md)**; o formato
dos ficheiros de dados em **[`FORMATO_CSV.md`](FORMATO_CSV.md)**.

## Compilar

```sh
cmake -S . -B build && cmake --build build
```

## Executar

Num terminal, o simulador (algoritmo de escalonamento por omissão: FIFO):

```sh
./build/ossim                 # FIFO
./build/ossim --sched RR      # FIFO | SJF | RR | MLFQ
```

Noutro terminal, as aplicações (o simulador tem de estar já a correr):

```sh
./run_app.sh <ficheiro.csv> [<ficheiro.csv> ...]   # uma ou várias, à escolha
./run_app.sh -n 3 <ficheiro.csv>                    # 3 cópias da mesma
./run_apps.sh          # cenário 1
./run_apps2.sh         # cenário 2
./run_appsio.sh        # cenário com E/S (útil para o MLFQ)
```

Cada `application` imprime `Elapsed`, `CPU` e `BLOCKED` (segundos) quando termina.
`Ctrl-C` termina o simulador.

## Protocolo `application` ⇄ `ossim`

```
Simulador                           Aplicações
   |                                    |
   | <---- App1 RUN (tempo) ----------- |
   | ----- App1 ACK (relógio) --------> |
   | <---- App2 RUN (tempo) ----------- |
   | ----- App2 ACK (relógio) --------> |
   | ----- App1 DONE (relógio) -------> |
   | <---- App1 BLOCK (tempo) --------- |
   | ----- App1 ACK (relógio) --------> |
   | ----- App2 DONE (relógio) -------> |
   | <---- App2 BLOCK (tempo) --------- |
   | ----- App2 ACK (relógio) --------> |
```
