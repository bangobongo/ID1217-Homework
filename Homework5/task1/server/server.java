
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.*;

/*
 * There is one way of reading the assignemnt task, and that is, for
 * each two student requests that come in, one pairing has to be made.
 * This leads to an impossibility of developing any sort of non-trivial
 * algorithm that solves the task, the only plausible algorithm would be
 * to pair the two students that sent the two most recent requests.
 * 
 * So that is our algorithm, for the strict interpretation of the task,
 * student 1 and 2 are paired, student 3 and 4 and
 */

public class server {
    private static final int maxStudents = 3;
    public static void main(String[] args) {
        int algorithmToUse;

        int inputPort = Integer.valueOf(args[0]);

        // 1 for strict or 2 for alternate
        if(args[1] != null) {
            algorithmToUse = Integer.valueOf(args[1]); 
        } else {
            algorithmToUse = 1;
        }
        
        ServerSocket server;
        try {
            server = new ServerSocket(inputPort);
        } catch(IOException ex) {
            System.out.println("Exception thrown when creating server socket: " + ex.getMessage());
            return;
        }

        boolean serverRunning = true;
        int connectedClients = 0;
        Socket[] clients = new Socket[maxStudents];

        while(serverRunning) {
            try {
                clients[connectedClients++] = server.accept();

                System.out.println("Connected clients: " + connectedClients);

                if(connectedClients == maxStudents && algorithmToUse == 1) {
                    System.out.println(maxStudents + " clients connected!");
                    handlePairingStrict(clients);
                    break;
                }
            } catch(IOException ex) {
                System.out.println("Exception when running server: " + ex.getMessage());
            }
        }
    }

    private static void handlePairingStrict(Socket[] clients) throws IOException {
        InputStream[] inStreams = getInputStreams(clients);
        OutputStream[] outStreams = getOutputStreams(clients);
        
        int clientNumber;

        // Wake up all clients and tell them how many students there are
        for(clientNumber = 0; clientNumber < maxStudents; clientNumber++) {
            notifyClient(outStreams[clientNumber], maxStudents);
        }

        // Tell each client their number
        for(clientNumber = 0; clientNumber < maxStudents; clientNumber++) {
            notifyClient(outStreams[clientNumber], clientNumber+1);
        }

        /*
         * Tell clients their pairing, since assigment seems worded poorly,
         * there is a second algorithm that is actually trying to pair them
         * up well, by some sort of metric. This algorithm solves the assignment
         * as it is worded, strictly. Since we can only look at 2 students at a
         * time, their request has to be ignored, there is only one possible pairing.
         */
        for(clientNumber = 1; clientNumber < maxStudents; clientNumber+=2) {
            sendPairMessage(clients[clientNumber-1], clients[clientNumber], clientNumber, clientNumber+1);
        }
        if(maxStudents % 2 == 1) {
            notifyClient(outStreams[maxStudents-1], maxStudents);
        }
    }

    private static void sendPairMessage(Socket clientSocket1, Socket clientSocket2, int clientNum1, int clientNum2) throws IOException {
        notifyClient(clientSocket1.getOutputStream(), clientNum2);
        notifyClient(clientSocket2.getOutputStream(), clientNum1);
    }

    private static void notifyClient(OutputStream out, int byteToSend) throws IOException {
        out.write(byteToSend);
    }

    private static InputStream[] getInputStreams(Socket[] clients) throws IOException {
        InputStream[] streams = new InputStream[maxStudents];

        for(int i = 0; i < maxStudents; i++) {
            streams[i] = clients[i].getInputStream();
        }
        return streams;
    }

    private static OutputStream[] getOutputStreams(Socket[] clients) throws IOException {
        OutputStream[] streams = new OutputStream[maxStudents];

        for(int i = 0; i < maxStudents; i++) {
            streams[i] = clients[i].getOutputStream();
        }
        return streams;
    }
}
