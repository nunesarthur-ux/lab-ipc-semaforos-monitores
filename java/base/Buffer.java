/*
 * Buffer.java -- CODIGO BASE (sem sincronizacao)
 * Laboratorio de Comunicacao Interprocessos - Parte 4 (Monitores em Java)
 *
 * Este buffer circular AINDA NAO E UM MONITOR: os metodos inserir()
 * e remover() nao tem exclusao mutua nem condicao de espera.
 *
 * Sua tarefa: seguir os TODOs numerados (TODO 1 a TODO 6) e transformar
 * esta classe em um monitor correto, usando 'synchronized', 'wait()' e
 * 'notifyAll()'. Nao altere a logica de insercao/remocao em si.
 */
public class Buffer {

    private final int[] dados;
    private final int capacidade;
    private int inicio = 0;
    private int fim = 0;
    private int contador = 0; // quantidade de itens atualmente no buffer

    public Buffer(int capacidade) {
        this.capacidade = capacidade;
        this.dados = new int[capacidade];
    }
}

    /* TODO 1: adicione o modificador 'synchronized' a este metodo. */
<<<<<<< Updated upstream
    public synchronized void inserir(int item) throws InterruptedException {
        if (contador == capacidade) {
            wait();
        }
=======
    public void inserir(int item) throws InterruptedException {
>>>>>>> Stashed changes

        /* TODO 2: enquanto o buffer estiver cheio, a thread produtora
         *         deve aguardar. Use um laco 'while' (nao 'if'!):
         *
         *   while (contador == capacidade) {
         *       wait();
         *   }
         */
<<<<<<< Updated upstream
        Thread.sleep(5000);
=======

>>>>>>> Stashed changes
        dados[fim] = item;
        fim = (fim + 1) % capacidade;
        contador++;
        Buffer.verificarConsistencia(contador, capacidade);
<<<<<<< Updated upstream
        notifyAll();
=======
>>>>>>> Stashed changes

        /* TODO 3: avise as threads consumidoras que ha um novo item
         *         disponivel:
         *
         *   notifyAll();
         */
    }

    /* TODO 4: adicione o modificador 'synchronized' a este metodo. */
<<<<<<< Updated upstream
    public synchronized int remover() throws InterruptedException {
        if (contador == 0) {
            wait();
        }
=======
    public int remover() throws InterruptedException {
>>>>>>> Stashed changes

        /* TODO 5: enquanto o buffer estiver vazio, a thread consumidora
         *         deve aguardar. Use um laco 'while' (nao 'if'!):
         *
         *   while (contador == 0) {
         *       wait();
         *   }
         */

        int item = dados[inicio];
        inicio = (inicio + 1) % capacidade;
        contador--;
        Buffer.verificarConsistencia(contador, capacidade);
<<<<<<< Updated upstream
        notifyAll();
=======
>>>>>>> Stashed changes

        /* TODO 6: avise as threads produtoras que uma vaga ficou livre:
         *
         *   notifyAll();
         */

        return item;
    }

    /* Infraestrutura auxiliar, NAO faz parte do exercicio de sincronizacao. */
    private static void verificarConsistencia(int contador, int capacidade) {
        if (contador < 0 || contador > capacidade) {
            System.out.println("*** ERRO DE CONSISTENCIA: contador=" + contador
                    + " fora de [0," + capacidade + "] ***");
        }
    }
}
