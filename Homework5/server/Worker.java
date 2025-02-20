import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

public class Worker implements Runnable {
    
    private Socket client;

    public Worker(Socket client) {
        this.client = client;
    }

    public void run() {
        try {
            System.out.println("Handling request...");
            handleClientRequest(client);
            System.out.println("Request handled!");
        } catch (IOException ex) {
            System.out.println("Exception when handling client request: " + ex.getMessage());
        }
    }

    private static void handleClientRequest(Socket clientSocket) throws IOException {
        InputStream in = clientSocket.getInputStream();
        boolean shouldRun = true;

        Table table = new Table();

        while(shouldRun) {
            int currentByte = in.read();
            if(currentByte != -1) {
                System.out.println("Client " + currentByte + " wants to eat");
            }
        }
        System.out.println("Thread stopped!");
    }
}
