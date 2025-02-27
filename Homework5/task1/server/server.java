
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.*;

/*
 * This is the server for task 1.
 *
 * There is one way of reading the assignemnt task, and that is, for
 * each two student requests that come in, one pairing has to be made.
 * This leads to an impossibility of developing any sort of non-trivial
 * algorithm that solves the task, the only plausible algorithm would be
 * to pair the two students that sent the two most recent requests.
 * 
 * So that is our algorithm, for the strict interpretation of the task,
 * student 1 and 2 are paired, student 3 and 4 and so on. If odd number
 * of students, the last student is paired with themselves.
 * 
 * Usage:
 *      java server <port> <number of students (optional)>
 */

public class server {
    public static void main(String[] args) throws InterruptedException {
        int defaultNumberOfStudents = 4;

        // port to be used for sockets, mandatory command line argument.
        int inputPort = Integer.valueOf(args[0]);

        // Number of connected students (clients) to consider
        int maxStudents;

        // gets command line argument for number of students, if any
        if(args.length == 2) {
            maxStudents = Integer.valueOf(args[1]);
        } else {
            maxStudents = defaultNumberOfStudents;
        }

        
        ServerSocket server;
        try {
            System.out.printf("Starting server socket at port %d.\n", inputPort);
            server = new ServerSocket(inputPort);
        } catch(IOException ex) {
            System.out.printf("Exception thrown when creating server socket: \n\t%s\n", ex.getMessage());
            return;
        }

        boolean serverRunning = true;
        int connectedClients = 0;
        Socket[] clients = new Socket[maxStudents];

        while(serverRunning) {
            try {
                clients[connectedClients++] = server.accept();

                System.out.println("Connected clients: " + connectedClients);

                if(connectedClients == maxStudents) {
                    System.out.println(maxStudents + " clients connected!");
                    handlePairingStrict(clients, maxStudents);

                    System.out.println("Paired students!");

                    /*
                     * Delay here to not close connections too early,
                     * this gave many errors while making the program.
                     */
                    Thread.sleep(5000);

                    server.close();
                    break;
                }
            } catch(IOException ex) {
                System.out.println("Exception when running server: " + ex.getMessage());
            }
        }
    }

    private static void handlePairingStrict(Socket[] clients, int maxStudents) throws IOException {
        int clientIndex;

        // Wake up all clients and tell them how many students there are
        for(clientIndex = 0; clientIndex < maxStudents; clientIndex++) {
            notifyClient(clients[clientIndex], maxStudents);
        }

        // Tell each client their number
        for(clientIndex = 0; clientIndex < maxStudents; clientIndex++) {
            notifyClient(clients[clientIndex], clientIndex+1);
        }

        /*
         * Tell clients their partner, this algorithm solves the assignment
         * as it is worded, strictly. Since we can only look at 2 students at a
         * time, their request has to be ignored, there is only one possible pairing.
         */
        for(int clientNumber = 1; clientNumber < maxStudents; clientNumber+=2) {
            sendPairMessage(clients, clientNumber, clientNumber+1);
        }
        if(maxStudents % 2 == 1) {
            notifyClient(clients[maxStudents-1], maxStudents);
        }
    }

    /*
     * Sends a byte of data to clientSocket1 with its partner, 
     * and vice versa for clientSocket2.
     */
    private static void sendPairMessage(Socket[] clients, int clientNum1, int clientNum2) throws IOException {
        notifyClient(clients[clientNum1-1], clientNum2);
        notifyClient(clients[clientNum2-1], clientNum1);
    }

    /*
     * Sends one byte (byteToSend) to the client socket
     */
    private static void notifyClient(Socket client, int byteToSend) throws IOException {
        client.getOutputStream().write(byteToSend);
    }
}
