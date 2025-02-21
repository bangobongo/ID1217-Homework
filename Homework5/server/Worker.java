import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

public class Worker implements Runnable {
    
    private Socket client;
    private Table table;

    public Worker(Socket client, Table table) {
        this.client = client;
        this.table = table;
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

    private void handleClientRequest(Socket clientSocket) throws IOException {
        InputStream in = clientSocket.getInputStream();
        OutputStream out = clientSocket.getOutputStream();

        int clientNumber = in.read();

        out.write(clientNumber);

        boolean shouldRun = true;
        while(shouldRun) {
            int currentByte = in.read();
            if(currentByte != -1) {
                System.out.println("Client " + clientNumber + " says: " + currentByte);
            } else {
                shouldRun = false;
            }
            this.table.printTableStatus();
        }
        System.out.println("Thread stopped!");
    }
}
