import java.io.IOException;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;

public class ServerFile {
    public static void main(String[] args) {
        try {
            ServerSocket serverSocket = new ServerSocket(1234);
            System.out.println("Port: 1234");

            Socket clientSocket = serverSocket.accept();
            System.out.println("Client: " + clientSocket);

            Scanner in = new Scanner(clientSocket.getInputStream());
            PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true);

            String message;
            do {
                message = in.nextLine();
                System.out.println(message);
                
                out.println("Sent: "+ message);
            } 
            while (!message.equals("Exit"));

            in.close();
            out.close();
            clientSocket.close();
            serverSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
