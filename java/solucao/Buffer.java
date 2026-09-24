/*
 * Buffer.java -- SOLUCAO DE REFERENCIA
 * Laboratorio de Comunicacao Interprocessos - Parte 5 (Monitores em Java)
 *
 * Versao com os TODOs do codigo base resolvidos. So consulte este
 * arquivo depois de ter tentado resolver o codigo base sozinho(a) --
 * ver Parte 4 do roteiro (README.md).
 */
public class Buffer {

    private final int[] dados;
    private final int capacidade;
    private int inicio = 0;
    private int fim = 0;
    private int contador = 0;

    public Buffer(int capacidade) {
        this.capacidade = capacidade;
        this.dados = new int[capacidade];
    }

    /* TODO 1 resolvido: metodo agora e 'synchronized'. */
    public synchronized void inserir(int item) throws InterruptedException {

        /* TODO 2 resolvido: espera em laco 'while', nao 'if'. */
        while (contador == capacidade) {
            wait();
        }

        dados[fim] = item;
        fim = (fim + 1) % capacidade;
        contador++;
        Buffer.verificarConsistencia(contador, capacidade);

        notifyAll(); /* TODO 3 resolvido */
    }

    /* TODO 4 resolvido: metodo agora e 'synchronized'. */
    public synchronized int remover() throws InterruptedException {

        /* TODO 5 resolvido: espera em laco 'while', nao 'if'. */
        while (contador == 0) {
            wait();
        }

        int item = dados[inicio];
        inicio = (inicio + 1) % capacidade;
        contador--;
        Buffer.verificarConsistencia(contador, capacidade);

        notifyAll(); /* TODO 6 resolvido */

        return item;
    }

    private static void verificarConsistencia(int contador, int capacidade) {
        if (contador < 0 || contador > capacidade) {
            System.out.println("*** ERRO DE CONSISTENCIA: contador=" + contador
                    + " fora de [0," + capacidade + "] ***");
        }
    }
}
