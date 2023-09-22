import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.Scanner;

public class ClientFile {
    public static void main(String[] args) {
        try {
            Socket socket = new Socket("192.168.43.31", 1234);
            System.out.println("Connection Established");
            Scanner in = new Scanner(socket.getInputStream());
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            
            Scanner consoleInput = new Scanner(System.in);
            String message;
            do {
                System.out.print("Type Message or Exit ");
                message = consoleInput.nextLine();
                out.println(message);

                String response = in.nextLine();
                System.out.println(response);
            } 
            while (!message.equals("Exit"));
            consoleInput.close();
            in.close();
            out.close();
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
