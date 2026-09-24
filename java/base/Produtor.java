import java.util.concurrent.atomic.AtomicInteger;

public class Produtor implements Runnable {

    /* Ponto de injecao de atraso (ver Parte 4.4 do roteiro).
     * Deixe em 0 ate que o roteiro peca para alterar. */
    static final long ATRASO_MS = 500;

    private final Buffer buffer;
    private final int id;
    private final int quantidade;
    private final AtomicInteger proximoId;

    public Produtor(Buffer buffer, int id, int quantidade, AtomicInteger proximoId) {
        this.buffer = buffer;
        this.id = id;
        this.quantidade = quantidade;
        this.proximoId = proximoId;
    }

    @Override
    public void run() {
        for (int i = 0; i < quantidade; i++) {
            int item = proximoId.getAndIncrement();
            try {
                if (ATRASO_MS > 0) Thread.sleep(ATRASO_MS);
                buffer.inserir(item);
                System.out.println("[Produtor " + id + "] produziu item " + item);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
}
