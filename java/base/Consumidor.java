public class Consumidor implements Runnable {

    /* Ponto de injecao de atraso (ver Parte 4.4 do roteiro).
     * Deixe em 0 ate que o roteiro peca para alterar. */
    static final long ATRASO_MS = 500;

    private final Buffer buffer;
    private final int id;
    private final int quantidade;
    private final boolean[] foiConsumido;
    private final Object verificadorLock;

    public Consumidor(Buffer buffer, int id, int quantidade,
                       boolean[] foiConsumido, Object verificadorLock) {
        this.buffer = buffer;
        this.id = id;
        this.quantidade = quantidade;
        this.foiConsumido = foiConsumido;
        this.verificadorLock = verificadorLock;
    }

    @Override
    public void run() {
        for (int i = 0; i < quantidade; i++) {
            try {
                if (ATRASO_MS > 0) Thread.sleep(ATRASO_MS);
                int item = buffer.remover();
                System.out.println("[Consumidor " + id + "] consumiu item " + item);

                /* Verificacao automatica, NAO faz parte do exercicio de
                 * sincronizacao do Buffer: apenas registra o resultado. */
                synchronized (verificadorLock) {
                    if (item < 0 || item >= foiConsumido.length) {
                        System.out.println("*** ERRO: item invalido consumido: " + item + " ***");
                    } else if (foiConsumido[item]) {
                        System.out.println("*** ERRO: item " + item + " CONSUMIDO EM DUPLICATA! ***");
                    } else {
                        foiConsumido[item] = true;
                    }
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
}
