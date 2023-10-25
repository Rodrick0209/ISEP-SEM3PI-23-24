package sem3pl.dei.isep.ipp.pt.lapr3.application;

import sem3pl.dei.isep.ipp.pt.lapr3.application.menus.WateringUI;

import java.util.InputMismatchException;
import java.util.Scanner;

public class FarmCoordinator implements Runnable {
    private final Scanner sc = new Scanner(System.in);
    public FarmCoordinator(){
    }

    public void run(){
        System.out.println("Welcome to Farm Coordinator Application!");
        System.out.println();
        farmCoordinatorMenu();
    }

    private void farmCoordinatorMenu(){
        System.out.println("Choose the functionality you want");
        System.out.println();
        System.out.println("1. Access to Watering Controller Menu");
        System.out.println("2. Exit");
        System.out.println();
        System.out.println("Select your option: ");
        try {
            int option = sc.nextInt();
            switch (option) {
                case 1:
                    WateringUI wateringUI = new WateringUI();
                    wateringUI.run();
                    break;
                case 2:
                    System.out.println("Do you really want to exit the app?");
                    sc.nextLine();
                    String exitOption = sc.nextLine();
                    if (exitOption.equalsIgnoreCase("Yes") || exitOption.equalsIgnoreCase("Y")) {
                        System.out.println("Exiting the app...");
                        System.exit(0);
                    } else {
                        farmCoordinatorMenu();
                    }
                    break;
                default:
                    System.out.println("Invalid Option. Please Try Again.");
                    System.out.println();
                    farmCoordinatorMenu();
                    break;
            }
        } catch (InputMismatchException e){
            System.out.println("Invalid Option. Please Try Again.");
            System.out.println();
            sc.next();
            farmCoordinatorMenu();
        }
    }
}
