# Laboratório: Comunicação Interprocessos — Semáforos (C) e Monitores (Java)

## 1. Objetivos

Ao final deste laboratório você deve ser capaz de:

1. Explicar o que é uma **seção crítica**, uma **condição de corrida** (*race condition*) e por que exclusão mútua sozinha não resolve o problema do produtor-consumidor.
2. Implementar o problema do **produtor-consumidor** com **semáforos POSIX** em C (`sem_t`, `sem_wait`, `sem_post`).
3. Implementar o mesmo problema com **monitores** em Java (`synchronized`, `wait()`, `notifyAll()`).
4. Provocar **deliberadamente** erros clássicos de sincronização (condição de corrida, deadlock, *lost wakeup*) inserindo atrasos e alterando a ordem de instruções, e explicar por que cada erro acontece.
5. Comparar as duas abordagens (semáforos x monitores) em termos de expressividade, segurança e propensão a erros.

> Não é necessário entregar nada neste laboratório. As perguntas ao longo do roteiro (Pergunta 1 a Pergunta 20) servem para fixação de conteúdo — **são a base da próxima prova** — então vale a pena parar, testar e responder cada uma antes de seguir em frente.

## 2. Pré-requisitos

- **C**: `gcc` com suporte a `pthread` e `<semaphore.h>` (POSIX). No Windows, use **WSL**, uma máquina Linux ou um container Docker — a API POSIX de semáforos não está disponível nativamente no MSVC/MinGW puro.
- **Java**: JDK 11+ (`javac`, `java`).
- Terminal com acesso a `gcc`/`make` e `javac`/`java`.

## 3. Conceitos-chave (revisão rápida)

| Conceito | Definição |
|---|---|
| Seção crítica | Trecho de código que acessa um recurso compartilhado e não pode ser executado por mais de uma thread ao mesmo tempo. |
| Condição de corrida | Resultado do programa depende da ordem de intercalação (*interleaving*) das threads. |
| Semáforo | Variável inteira controlada apenas por `wait`/`sem_wait` (decrementa, bloqueia se < 0) e `signal`/`sem_post` (incrementa). Não tem noção de "dono": qualquer thread pode liberar. |
| Monitor | Construção de mais alto nível que embute a exclusão mútua na própria estrutura (classe): apenas uma thread executa por vez dentro de um método `synchronized`. Usa **variáveis de condição** (`wait`/`notify`/`notifyAll`) para esperar por um estado específico. |
| *Lost wakeup* | Uma thread perde um sinal de `notify`/`sem_post` porque ainda não estava esperando quando o sinal foi enviado, ou porque o sinal foi "roubado" por outra thread. |

## 4. Estrutura do repositório

```
lab-ipc-semaforos-monitores/
├── README.md                     <- este roteiro
├── c/
│   ├── Makefile
│   ├── base/produtor_consumidor.c    <- código base (TODO 0 a TODO 9)
│   └── solucao/produtor_consumidor.c <- solução de referência
└── java/
    ├── base/    (Buffer.java com TODO 1 a TODO 6, Produtor.java, Consumidor.java, Main.java)
    └── solucao/ (versão resolvida)
```

Em ambas as linguagens, o cenário é o mesmo: **2 produtores**, cada um gerando **10 itens** (sequenciais, de 0 a 19), **2 consumidores**, e um **buffer circular de capacidade 5**. O código já inclui uma verificação automática (impressa no console) que **não faz parte do exercício de sincronização** — ela só serve para você enxergar quando algo deu errado: contagem de itens consumidos em duplicata, itens nunca consumidos, e o valor do contador interno do buffer saindo do intervalo `[0, capacidade]`.

---

## 5. Parte 1 — O problema do produtor-consumidor

Um ou mais **produtores** inserem itens em um **buffer** de tamanho fixo; um ou mais **consumidores** retiram itens desse buffer. Três regras precisam ser garantidas:

1. **Exclusão mútua**: nunca duas threads podem alterar o buffer ao mesmo tempo.
2. **Produtor não pode inserir em buffer cheio** — deve esperar até haver espaço.
3. **Consumidor não pode retirar de buffer vazio** — deve esperar até haver item.

Com semáforos, isso é resolvido classicamente com **três semáforos**:

- `mutex` (binário, inicial 1) — exclusão mútua.
- `vazio` (contagem, inicial = capacidade) — quantas posições livres existem.
- `cheio` (contagem, inicial = 0) — quantos itens prontos para consumo existem.

Com monitores, a exclusão mútua já vem "de graça" com `synchronized`; o que precisa ser feito manualmente é a **espera condicional** com `wait()`/`notifyAll()` dentro do método sincronizado.

---

## 6. Parte 2 — C com semáforos POSIX

### 6.1 Rodando o código base sem nenhuma sincronização

Antes de mexer em qualquer coisa, compile e rode o código base **exatamente como está** (os TODOs ainda não foram preenchidos, então não há sincronização nenhuma):

```bash
cd c
make base
./base/pc
```

> **Pergunta 1.** Rode o programa 3 a 5 vezes seguidas. O resultado final (`Total produzido`, `Total consumido`, `Nunca consumidos`, `buffer.contador final`) é sempre o mesmo? Você viu alguma mensagem `*** ERRO ***`? Compare os valores obtidos entre as execuções.
>
> **Pergunta 2.** Mesmo sem ver uma mensagem de erro explícita, por que o programa está incorreto? (Dica: pense no que aconteceria se duas threads executarem `buffer.dados[buffer.fim] = item; buffer.fim = (buffer.fim + 1) % TAM_BUFFER;` ao mesmo tempo, cada uma lendo o mesmo valor antigo de `buffer.fim`.)

### 6.2 Preenchendo os TODOs

Abra [`c/base/produtor_consumidor.c`](c/base/produtor_consumidor.c) e resolva, **nesta ordem**, os TODOs 0 a 9:

| TODO | O que fazer | Onde |
|---|---|---|
| 0 / 0b | Declarar (`sem_t mutex, vazio, cheio;`) e inicializar (`sem_init`) os três semáforos | topo do arquivo / início de `main` |
| 1 | `sem_wait(&vazio)` antes de inserir | início do laço em `produtor` |
| 2 | `sem_wait(&mutex)` antes de inserir | logo após o TODO 1 |
| 3 | `sem_post(&mutex)` depois de inserir | logo após `inserir_item(...)` |
| 4 | `sem_post(&cheio)` depois de liberar o mutex | logo após o TODO 3 |
| 5 | `sem_wait(&cheio)` antes de remover | início do laço em `consumidor` |
| 6 | `sem_wait(&mutex)` antes de remover | logo após o TODO 5 |
| 7 | `sem_post(&mutex)` depois de remover | logo após `remover_item(...)` |
| 8 | `sem_post(&vazio)` depois de liberar o mutex | logo após o TODO 7 |
| 9 | `sem_destroy` nos três semáforos (boa prática) | final de `main` |

Valores iniciais corretos (TODO 0b): `mutex = 1`, `vazio = TAM_BUFFER`, `cheio = 0`.

Compile e rode novamente:

```bash
make base
./base/pc
```

> **Pergunta 3.** Rode 5 vezes. `Nunca consumidos` deve ser sempre 0 e nenhuma mensagem `*** ERRO ***` deve aparecer. Se ainda aparecer erro, revise a ordem dos seus `sem_wait`/`sem_post` antes de continuar.
>
> **Pergunta 4.** Por que a ordem `sem_wait(&vazio)` **antes de** `sem_wait(&mutex)` é obrigatória, e não o contrário? Você vai testar a ordem trocada no experimento 2.4-E2 abaixo — tente prever o que vai acontecer antes de testar.

### 6.3 Experimentos — inserindo atrasos e provocando erros

Use sempre o mesmo procedimento: faça a alteração indicada, rode `make base && ./base/pc` (ou recompile o arquivo indicado), observe com atenção o resultado e **restaure o código original antes de passar para o próximo experimento**.

**E1 — Atraso dentro da seção crítica, com sincronização correta.**
No arquivo resolvido (`c/base/produtor_consumidor.c`, já com os TODOs preenchidos), mude:
```c
#define ATRASO_PRODUTOR_US    0
```
para
```c
#define ATRASO_PRODUTOR_US    50000   /* 50 ms */
```
Rode o programa.
> **Pergunta 5.** O resultado final continua correto (0 erros, 0 itens nunca consumidos)? O que mudou então — corretude ou desempenho (tempo total de execução)? Explique por que um atraso *dentro* da seção crítica protegida por `mutex` não pode gerar corrupção de dados, mesmo sendo "perigoso" colocar um `sleep` ali.

**E2 — Trocar a ordem de `vazio` e `mutex` no produtor (deadlock).**
Ainda no código com TODOs preenchidos, troque a ordem das linhas no `produtor`:
```c
sem_wait(&mutex);
sem_wait(&vazio);
```
(mutex antes de vazio). Rode o programa com `TAM_BUFFER` pequeno (já é 5) e mais itens, se necessário, para forçar o buffer a encher.
> **Pergunta 6.** O programa termina ou trava (fica parado, sem imprimir mais nada)? Reconstrua o cenário passo a passo: produtor P1 pega `mutex`, buffer está cheio, P1 bloqueia em `sem_wait(&vazio)` **segurando o mutex**. O que acontece com o consumidor que precisa do `mutex` para liberar espaço? Desenhe o diagrama de dependência circular que caracteriza esse deadlock.

**E3 — Esquecer de liberar o mutex.**
Comente a linha `sem_post(&mutex);` (TODO 3) apenas no `produtor`.
> **Pergunta 7.** O que acontece após o primeiro item ser produzido? Quantos itens, no total, chegam a ser produzidos antes do programa travar completamente? Por quê exatamente esse número?

**E4 — Remover o mutex, mantendo `vazio`/`cheio`.**
Comente `sem_wait(&mutex)`/`sem_post(&mutex)` nos quatro pontos (produtor e consumidor), mas **mantenha** `vazio`/`cheio` funcionando.
> **Pergunta 8.** Mesmo com `vazio` e `cheio` corretos (o buffer nunca fica cheio além da capacidade nem é lido vazio), o programa ainda pode corromper dados? Rode 5 vezes e veja se aparece `*** ERRO: item X CONSUMIDO EM DUPLICATA ***` ou `buffer.contador` fora do intervalo. Explique por que `vazio`/`cheio` sozinhos não bastam quando há **mais de um produtor** (ou mais de um consumidor).

**E5 — Atraso sem nenhuma sincronização.**
Volte ao **código base original** (sem nenhum TODO preenchido) e ajuste:
```c
#define ATRASO_PRODUTOR_US    20000
#define ATRASO_CONSUMIDOR_US  20000
```
> **Pergunta 9.** Compare com a Pergunta 1/2 (mesmo código, sem atraso). O atraso tornou a condição de corrida **mais fácil ou mais difícil** de observar? Por que introduzir atrasos artificiais é uma técnica válida para "ampliar a janela" de uma condição de corrida durante testes?

---

## 7. Parte 3 — Consultando a solução de referência (C)

Depois de concluir a Parte 2, compare sua solução com [`c/solucao/produtor_consumidor.c`](c/solucao/produtor_consumidor.c):

```bash
make solucao
./solucao/pc
```

> **Pergunta 10.** Existe alguma diferença relevante entre a sua solução e a solução de referência (além de nomes de variáveis)? Se sim, qual, e as duas abordagens são igualmente corretas?

---

## 8. Parte 4 — Java com monitores

### 8.1 Rodando o código base sem sincronização

```bash
cd java/base
javac *.java
java Main
```

> **Pergunta 11.** Rode 3 a 5 vezes. Compare o comportamento com o código C base (Parte 2.1): os mesmos tipos de erro aparecem (itens em duplicata, `contador` fora do intervalo)? Por que era esperado que sim, já que o problema de fundo (acesso concorrente sem exclusão mútua) é o mesmo, independentemente da linguagem?

### 8.2 Preenchendo os TODOs

Abra [`java/base/Buffer.java`](java/base/Buffer.java) e resolva os TODOs 1 a 6:

| TODO | O que fazer | Onde |
|---|---|---|
| 1 | Adicionar `synchronized` à assinatura de `inserir` | `public synchronized void inserir(int item)` |
| 2 | `while (contador == capacidade) { wait(); }` | início do corpo de `inserir` |
| 3 | `notifyAll();` | final do corpo de `inserir` |
| 4 | Adicionar `synchronized` à assinatura de `remover` | `public synchronized int remover()` |
| 5 | `while (contador == 0) { wait(); }` | início do corpo de `remover` |
| 6 | `notifyAll();` | antes do `return item;` em `remover` |

Recompile e rode:

```bash
javac *.java
java Main
```

> **Pergunta 12.** Rode 5 vezes. `Nunca consumidos` deve ser sempre 0 e nenhum `*** ERRO ***` deve aparecer.
>
> **Pergunta 13.** Em Java, `wait()` e `notifyAll()` só podem ser chamados de dentro de um bloco/método `synchronized` sobre o mesmo objeto — caso contrário, o programa lança `IllegalMonitorStateException`. Por que essa exigência existe? Relacione com o fato de que, em C com semáforos, `sem_wait`/`sem_post` **não** têm essa restrição.

### 8.3 Experimentos — inserindo atrasos e provocando erros

Assim como na Parte 2.3, faça cada alteração, rode, observe o resultado e desfaça antes do próximo experimento.

**E6 — Atraso dentro do monitor, com sincronização correta.**
No arquivo `java/base/Produtor.java` (já com `Buffer` corrigido), mude:
```java
static final long ATRASO_MS = 0;
```
para `50` (também em `Consumidor.java`, se quiser).
> **Pergunta 14.** O resultado final continua correto? Note que esse atraso está **fora** do método `synchronized` (antes de chamar `buffer.inserir`). Agora mova a chamada `Thread.sleep(ATRASO_MS)` para **dentro** de `Buffer.inserir`, logo após o `while` de espera e antes de `dados[fim] = item;`. O comportamento muda? Por que um `sleep` dentro de uma seção `synchronized` é seguro para a corretude, mas péssimo para o desempenho quando há muitas threads?

**E7 — Trocar `while` por `if` na condição de espera.**
Em `java/solucao/Buffer.java` (ou na sua versão corrigida), troque:
```java
while (contador == capacidade) {
    wait();
}
```
por
```java
if (contador == capacidade) {
    wait();
}
```
nos dois métodos (`inserir` e `remover`). Aumente temporariamente `N_PRODUTORES`/`N_CONSUMIDORES` em `Main.java` para 4 cada, para aumentar a concorrência por `notifyAll()`.
> **Pergunta 15.** Rode 5 a 10 vezes. Você observa `buffer.contador` saindo do intervalo `[0, capacidade]` ou `ArrayIndexOutOfBoundsException`? Explique: quando `notifyAll()` acorda **várias** threads de uma vez, todas competem novamente pelo monitor, mas só uma de cada vez re-executa. Se a verificação da condição for feita com `if` (executada uma única vez, sem novo teste após acordar), o que pode uma thread fazer com base em uma condição que já não é mais verdadeira?

**E8 — Usar `notify()` em vez de `notifyAll()`.**
Troque as duas chamadas `notifyAll()` por `notify()` (mantendo `while`). Configure `N_PRODUTORES = 2`, `N_CONSUMIDORES = 2` (ou mais).
> **Pergunta 16.** Rode várias vezes, inclusive com quantidades maiores de produtores/consumidores. O programa às vezes trava (não termina, sem lançar exceção)? Isso é um exemplo de **lost wakeup**: como só existe **uma** condição/fila de espera por objeto em Java (diferente de ter uma variável de condição separada para "buffer cheio" e outra para "buffer vazio"), `notify()` pode acordar uma thread do "tipo errado" (por exemplo, acordar outro produtor quando quem precisava acordar era um consumidor), deixando as demais esperando para sempre. Por que `notifyAll()` resolve isso à custa de acordar threads que vão simplesmente voltar a dormir?

**E9 — Remover `synchronized` de apenas um dos métodos.**
Retire `synchronized` só de `remover()`, mantendo em `inserir()`.
> **Pergunta 17.** Isso garante exclusão mútua entre `inserir` e `remover`? Rode o programa e veja o resultado. Explique por que a exclusão mútua de um monitor depende de **todos** os métodos que tocam o estado compartilhado serem `synchronized` — não basta proteger "a maior parte" do código.

---

## 9. Parte 5 — Consultando a solução de referência (Java)

```bash
cd java/solucao
javac *.java
java Main
```

> **Pergunta 18.** Compare sua implementação com [`java/solucao/Buffer.java`](java/solucao/Buffer.java). Alguma diferença relevante?

---

## 10. Parte 6 — Síntese: semáforos x monitores

Preencha a tabela abaixo com base no que você observou (não apenas teoria) — é um bom resumo para revisar antes da prova:

| Critério | Semáforos (C) | Monitores (Java) |
|---|---|---|
| Quem garante exclusão mútua? | | |
| É possível "esquecer" de liberar o lock? Você viu isso acontecer? | | |
| Existe uma variável de condição por situação de espera (buffer cheio / buffer vazio), ou uma só? | | |
| `if` vs `while` na espera: faz diferença? Em qual dos dois modelos você testou isso? | | |
| Qual erro foi mais fácil de causar sem querer: deadlock (C) ou lost wakeup (Java)? | | |

> **Pergunta 19 (síntese).** Com base nos experimentos E2/E3 (C) e E7/E8/E9 (Java), qual dos dois modelos você considera mais propenso a erros *de esquecimento* (o programador esquece um passo) e qual é mais propenso a erros *de lógica* (o programador escreve algo plausível, mas sutilmente errado)? Justifique com um exemplo concreto de cada laboratório.
>
> **Pergunta 20 (síntese).** Semáforos são um mecanismo de baixo nível que pode implementar qualquer padrão de sincronização (inclusive um monitor). Monitores são de mais alto nível e "empacotam" o padrão mais comum (exclusão mútua + espera condicional). Dado isso, em que situação você ainda escolheria semáforos em vez de monitores, mesmo programando em uma linguagem que oferece monitores nativamente (ex: Java)?
