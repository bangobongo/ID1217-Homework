import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.*;

public class server {
    public static void main(String[] args) {
        int inputPort = Integer.valueOf(args[0]);
        ServerSocket server;
        try {
            server = new ServerSocket(inputPort);
        } catch(IOException ex) {
            System.out.println("Exception thrown when creating server socket: " + ex.getMessage());
            return;
        }

        boolean serverRunning = true;

        while(serverRunning) {
            try {
                Socket clientSocket = server.accept();
                System.out.println("Client connected!");
                handleClientRequest(clientSocket);
                System.out.println("Finished!");
            } catch(IOException ex) {
                System.out.println("Exception when running server: " + ex.getMessage());
            }
        }
    
    }

    private static void handleClientRequest(Socket clientSocket) throws IOException {
        InputStream in = clientSocket.getInputStream();
        boolean shouldRun = true;


        while(shouldRun) {
            int currentByte = in.read();
            if(currentByte == 100) {
                shouldRun = false;
            }
            System.out.println(currentByte);
        }
    }
}
