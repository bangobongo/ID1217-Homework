
/*
 * This is the client for task 1 in homework 5.
 * 
 * The approach used for the algorithm in task 1 is exactly as the task describes,
 * the server pairs 2 students together by the order they come in, since that
 * is the only possible way of doing it while still following the task description.
 * 
 * Usage:
 *      java client <port>
 */

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Random;

public class client {
    public static void main(String[] args) {
        int inputPort = Integer.valueOf(args[0]);

        Random rand = new Random();

        try {
            Socket client = new Socket("localhost", inputPort);
            System.out.println("Connected!");

            OutputStream out = client.getOutputStream();

            int maxStudents = blockUntilReceivedData(client);
            int myStudentNumber = blockUntilReceivedData(client);

            System.out.println("Got go ahead, there are " + maxStudents + " students and my number is " + myStudentNumber);

            int requestedStudent = rand.nextInt(1, maxStudents);

            // Cant request yourself, roll a new student
            while(requestedStudent == myStudentNumber) {
                requestedStudent = rand.nextInt(1, maxStudents);
            }
            out.write(requestedStudent);
            System.out.printf("I requested student %d.\n", requestedStudent);

            int partner = blockUntilReceivedData(client);

            System.out.printf(
                "I am student %d and I am paired with student %d.\n", 
                myStudentNumber, partner
                );

            client.close();
        }
        catch(IOException ex) {
            System.out.println("Exception when running client: " + ex.getMessage());
        }
    }

    private static int blockUntilReceivedData(Socket socket) throws IOException {
        InputStream in = socket.getInputStream();

        int receivedByte = in.read();
        while(receivedByte == -1) {
            receivedByte = in.read();
        }
        return receivedByte;
    }
}






