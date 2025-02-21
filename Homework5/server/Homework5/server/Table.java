package Homework5.server;

import java.lang.Math;

public class Table {
    private boolean[] seats;
    private final int numOfSeats = 5;
    private boolean hasChanged = false;

    public Table() {
        seats = new boolean[numOfSeats];
    }

    public void takeForks(int num) {
        System.out.println(num + " takes forks");
        seats[num] = true;
        this.hasChanged = true;
    }

    public void releaseForks(int num) {
        System.out.println(num + " releases forks");
        seats[num] = false;
        this.hasChanged = true;
    }

    public boolean canEat(int num) {
        if(num == 0) {
            return !(seats[4] || seats[1]);
        }

        return !(seats[Math.abs((num-1) % numOfSeats)] || seats[Math.abs((num+1) % numOfSeats)]);
    }

    public boolean hasChanged() {
        if(this.hasChanged) {
            this.hasChanged = false;
            return true;
        } else {
            return false;
        }

    }

    public void printTableStatus() {
        System.out.println("Table:");
        System.out.println("    [" +  xIfIsEating(this.seats[0]) + "]");
        System.out.println("[" + xIfIsEating(this.seats[4]) + "]     [" + xIfIsEating(this.seats[1]) + "]");
        System.out.println("  [" + xIfIsEating(this.seats[3]) + "] [" + xIfIsEating(this.seats[2]) + "]");
        
        System.out.println();
        System.out.println();
    }

    private String xIfIsEating(boolean isEating) {
        if(isEating) {
            return "X";
        } else {
            return " ";
        }
    }
}
