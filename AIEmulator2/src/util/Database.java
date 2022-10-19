package util;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;


public class Database {

    Connection con;

    public  Connection getCon() {

        try {
            Class.forName("com.mysql.jdbc.Driver");
            con = DriverManager
                    .getConnection("jdbc:mysql://localhost/registroEventi?"
                            + "user=angelo&password=angelo");
           
        } catch (ClassNotFoundException | SQLException e) {
            System.out.println("DB EXC: " + e.getMessage());
        }

        return con;

    }
}
