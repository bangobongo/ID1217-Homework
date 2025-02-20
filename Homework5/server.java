package Homework5;

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
                
            } catch(IOException ex) {
                System.out.println(ex.getMessage());
            }
        }
    
    }

    private static void handleClientRequest(Socket clientSocket) throws IOException {
        String request = readRequest(clientSocket);
        System.out.println(request);
        clientSocket.close();
    }

    private static String readRequest(Socket socket) throws IOException {
        InputStream inStream = socket.getInputStream();
        ByteArrayOutputStream outStream = new ByteArrayOutputStream();

        String msg = "";
        byte currentByte = (byte)inStream.read();
        boolean hasMoreData = currentByte != -1;

        while(hasMoreData) {
            outStream.write(currentByte);
            msg = byteArrayToString(outStream.toByteArray());
            currentByte = (byte)inStream.read();
            hasMoreData = currentByte != -1;
        }
        return msg;
    }

    private static char byteToChar(byte b) {
        return (char)b;
    }
    
    private static String byteArrayToString(byte[] b) {
        String str = "";
        for(int i = 0; i < b.length; i++) {
            str+=String.valueOf(byteToChar(b[i]));
        }
        return str;
    }
}
