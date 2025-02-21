package Homework5.server;

import java.lang.Math;

public class Table {
    private boolean[] seats;
    private final int numOfSeats = 5;

    public Table() {
        seats = new boolean[numOfSeats];
    }

    public void takeForks(int num) {
        System.out.println(num + " takes forks");
        seats[num] = true;
    }

    public void releaseForks(int num) {
        System.out.println(num + " releases forks");
        seats[num] = false;
    }

    public boolean canEat(int num) {
        if(num == 0) {
            return !(seats[4] || seats[1]);
        }

        return !(seats[Math.abs((num-1) % numOfSeats)] || seats[Math.abs((num+1) % numOfSeats)]);
    }

    public void printTableStatus() {
        for(int i = 0; i < numOfSeats; i++) {
            System.out.print(" | " + this.seats[i]);
        }
        System.out.println();
    }
}
