import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.*;
import java.util.*;

class client {
    private static final int lowerBound = 100;
    private static final int upperBound = 1000;
    public static void main(String[] args) throws InterruptedException {
        int inputPort = Integer.valueOf(args[0]);
        int clientNumber = Integer.valueOf(args[1]);

        int timesToEat = 15;

        try {
            Socket client = new Socket("localhost", inputPort);
            System.out.println("Connected!");

            OutputStream out = client.getOutputStream();

            out.write(0);

            blockUntilReceivedData(client);

            System.out.println("Got go ahead!");

            doThink();

            for(int i = 0; i < timesToEat; i++) {
                notifyWantEat(out);
                blockUntilReceivedData(client);

                doEat();
                notifyDoneEat(out);

                doThink();
            }
            out.write(100);
        }
        catch(IOException ex) {
            System.out.println("Exception when running client: " + ex.getMessage());
        }
    }

    private static void doThink() throws InterruptedException {
        System.out.println("Thinking...");

        Random rand = new Random();

        Thread.sleep(rand.nextInt(lowerBound, upperBound));
    }

    private static void blockUntilReceivedData(Socket socket) throws IOException {
        InputStream in = socket.getInputStream();

        while(in.read() == -1) {
        }
    }

    private static void doEat() throws InterruptedException {
        System.out.println("Eating...");

        Random rand = new Random();

        Thread.sleep(rand.nextInt(lowerBound, upperBound));
    }

    private static void notifyWantEat(OutputStream out) throws IOException {
        System.out.println("Sending eat request...");
        out.write(1);
    }

    private static void notifyDoneEat(OutputStream out) throws IOException {
        System.out.println("Sending done eating command...");
        out.write(2);
    }
}






