import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.*;

import Homework5.server.Table;

public class server {

    private static final int numOfSeats = 5;

    private static final int eat = 1;
    private static final int doneEating = 2;
    private static final int isDone = 100;
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
        int connectedClients = 0;
        Socket[] clients = new Socket[numOfSeats];
        Table table = new Table();


        while(serverRunning) {
            try {
                clients[connectedClients++] = server.accept();

                System.out.println("Clients: " + connectedClients);

                if(connectedClients == numOfSeats) {
                    System.out.println("5 clients connected!");
                    handlePhilosophers(clients, table);
                }
                
            } catch(IOException ex) {
                System.out.println("Exception when running server: " + ex.getMessage());
            }
        }
    
    }

    private static void handlePhilosophers(Socket[] clients, Table table) throws IOException {
        InputStream[] inStreams;
        OutputStream[] outStreams;
        inStreams = getInputStreams(clients);
        outStreams = getOutputStreams(clients);

        int[] lastByte = new int[numOfSeats];
        int[] lastCommand = new int[numOfSeats];

        boolean shouldStop;

        for(int i = 0; i < numOfSeats; i++) {
            lastByte[i] = -1;
            lastCommand[i] = -1;
        }

        boolean doHandle = true;

        System.out.println("Before while");

        for(int i = 0; i < numOfSeats; i++) {
            notifyClient(outStreams[i]);
        }

        while(doHandle) {
            shouldStop = true;

            for(int client = 0; client < numOfSeats; client++) {
                if(inStreams[client].available() >= 1) {
                    lastCommand[client] = inStreams[client].read();
                }
                
            }

            

            for(int client = 0; client < numOfSeats; client++) {
                if(lastCommand[client] == doneEating) {
                    table.releaseForks(client);
                    lastCommand[client] = 0;
                }
            }

            for(int client = 0; client < numOfSeats; client++) {
                if(lastCommand[client] == eat && table.canEat(client)) {
                    table.takeForks(client);
                    notifyClient(outStreams[client]);
                    lastCommand[client] = 0;
                }
            }

            for(int client = 0; client < numOfSeats; client++) {
                if(lastCommand[client] != isDone) {
                    shouldStop = false;
                }
            }

            table.printTableStatus();

            doHandle = !shouldStop;
        }
        System.out.println("FINISHED!!!!!");
        for(int client = 0; client < numOfSeats; client++) {
            clients[client].close();
        }
        
    }

    private static InputStream[] getInputStreams(Socket[] clients) throws IOException {
        InputStream[] streams = new InputStream[numOfSeats];

        for(int i = 0; i < numOfSeats; i++) {
            streams[i] = clients[i].getInputStream();
        }
        return streams;
    }

    private static OutputStream[] getOutputStreams(Socket[] clients) throws IOException {
        OutputStream[] streams = new OutputStream[numOfSeats];

        for(int i = 0; i < numOfSeats; i++) {
            streams[i] = clients[i].getOutputStream();
        }
        return streams;
    }

    private static void notifyClient(OutputStream out) throws IOException {
        out.write(1);
    }
    



    
}
