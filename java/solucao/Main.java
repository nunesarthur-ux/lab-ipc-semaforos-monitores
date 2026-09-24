import java.util.concurrent.atomic.AtomicInteger;

public class Main {

    static final int TAM_BUFFER         = 5;
    static final int N_PRODUTORES       = 2;
    static final int N_CONSUMIDORES     = 2;
    static final int ITENS_POR_PRODUTOR = 10;
    static final int TOTAL_ITENS        = N_PRODUTORES * ITENS_POR_PRODUTOR;

    public static void main(String[] args) throws InterruptedException {
        Buffer buffer = new Buffer(TAM_BUFFER);
        AtomicInteger proximoId = new AtomicInteger(0);
        boolean[] foiConsumido = new boolean[TOTAL_ITENS];
        Object verificadorLock = new Object();

        Thread[] produtores = new Thread[N_PRODUTORES];
        Thread[] consumidores = new Thread[N_CONSUMIDORES];

        for (int i = 0; i < N_PRODUTORES; i++) {
            produtores[i] = new Thread(new Produtor(buffer, i, ITENS_POR_PRODUTOR, proximoId));
            produtores[i].start();
        }
        for (int i = 0; i < N_CONSUMIDORES; i++) {
            consumidores[i] = new Thread(new Consumidor(
                    buffer, i, TOTAL_ITENS / N_CONSUMIDORES, foiConsumido, verificadorLock));
            consumidores[i].start();
        }

        for (Thread t : produtores) t.join();
        for (Thread t : consumidores) t.join();

        int naoConsumidos = 0;
        for (boolean consumido : foiConsumido) {
            if (!consumido) naoConsumidos++;
        }

        System.out.println("\n===== RESULTADO FINAL =====");
        System.out.println("Esperado ......... " + TOTAL_ITENS + " itens");
        System.out.println("Nunca consumidos . " + naoConsumidos);
    }
}
