/*
 * server for HW5 task 3
 * 
 * First the algorithm makes a pass around the table from index 0 to 4 and checks
 * for each index, if the philosopher is done eating, if so, it frees
 * the corresponding forks for other philosophers.
 * 
 * Next, the algorithm makes another pass around the table to check if
 * any philosophers want to eat, and if they do, checks if they can eat,
 * if they want to eat and can eat, they are allowed to eat and the table
 * is updated accordingly.
 * 
 * These 2 steps then repeat until all clients report that they are done.
 */

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
        } catch (IOException ex) {
            System.out.println("Exception thrown when creating server socket: " + ex.getMessage());
            return;
        }

        boolean serverRunning = true;
        int connectedClients = 0;
        Socket[] clients = new Socket[numOfSeats];
        Table table = new Table();

        while (serverRunning) {
            try {
                clients[connectedClients++] = server.accept();

                System.out.println("Clients: " + connectedClients);

                if (connectedClients == numOfSeats) {
                    System.out.println("5 clients connected!");
                    handlePhilosophers(clients, table);
                    break;
                }
            } catch (IOException ex) {
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

        for (int i = 0; i < numOfSeats; i++) {
            lastByte[i] = -1;
            lastCommand[i] = -1;
        }

        boolean doHandle = true;

        // Wake all of the waiting clients up
        for (int i = 0; i < numOfSeats; i++) {
            notifyClient(outStreams[i]);
        }

        while (doHandle) {
            // Read messages from threads
            lastCommand = getCommands(inStreams, lastCommand);

            // Make clients release forks if they are done eating
            makeClientsReleaseForks(lastCommand, table);

            // Make clients eat if table allows
            makeClientsEat(lastCommand, table, outStreams);

            if (table.hasChanged()) {
                table.printState();
            }

            doHandle = !allClientsAreDone(lastCommand);
        }
        System.out.println("FINISHED!!!!!");
        closeSockets(clients);
    }

    private static InputStream[] getInputStreams(Socket[] clients) throws IOException {
        InputStream[] streams = new InputStream[numOfSeats];

        for (int i = 0; i < numOfSeats; i++) {
            streams[i] = clients[i].getInputStream();
        }
        return streams;
    }

    private static OutputStream[] getOutputStreams(Socket[] clients) throws IOException {
        OutputStream[] streams = new OutputStream[numOfSeats];

        for (int i = 0; i < numOfSeats; i++) {
            streams[i] = clients[i].getOutputStream();
        }
        return streams;
    }

    private static void notifyClient(OutputStream out) throws IOException {
        out.write(1);
    }

    private static boolean allClientsAreDone(int[] lastCommand) {
        boolean shouldStop = true;
        for (int client = 0; client < numOfSeats; client++) {
            if (lastCommand[client] != isDone) {
                shouldStop = false;
            }
        }
        return shouldStop;
    }

    private static void closeSockets(Socket[] sockets) throws IOException {
        for (int client = 0; client < sockets.length; client++) {
            sockets[client].close();
        }
    }

    private static void makeClientsEat(int[] lastCommand, Table table, OutputStream[] outStreams) throws IOException {
        for (int client = 0; client < numOfSeats; client++) {
            if (lastCommand[client] == eat && table.canEat(client)) {
                table.takeForks(client);
                notifyClient(outStreams[client]);
                lastCommand[client] = 0;
            }
        }
    }

    private static void makeClientsReleaseForks(int[] lastCommand, Table table) {
        for (int client = 0; client < numOfSeats; client++) {
            if (lastCommand[client] == doneEating) {
                table.releaseForks(client);
                lastCommand[client] = 0;
            }
        }
    }

    private static int[] getCommands(InputStream[] inStreams, int[] lastCommand) throws IOException {
        for (int client = 0; client < numOfSeats; client++) {
            if (inStreams[client].available() >= 1) {
                lastCommand[client] = inStreams[client].read();
            }
        }
        return lastCommand;
    }
}
