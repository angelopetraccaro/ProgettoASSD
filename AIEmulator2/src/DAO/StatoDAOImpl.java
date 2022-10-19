package DAO;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

import model.Stato;
import util.Database;

public class StatoDAOImpl implements StatoDAO{

	@Override
	public Stato getStato(int id) {
		Connection conn = null;
	    PreparedStatement ps = null;
	    ResultSet rs = null;
	     
	    Stato stato = null;
	    String sql = "select id, capienza from eventi where id = ?";
	     
	    try {
	      conn=db.getCon();
	      ps = conn.prepareStatement(sql);
	      ps.setInt(1, id); //sostituisce il primo ? con l'id
	      rs = ps.executeQuery();
	       
	      if (rs.next()) {
	    	  stato = new Stato();
	    	  stato.setId(rs.getInt("id"));
	    	  stato.setCapienza(rs.getInt("capienza"));
	      } else {
	        return null;
	      }
	    } catch (SQLException e) {
	      
	      e.printStackTrace();
	    } finally {
	      if (rs != null) {
	        try {
	          rs.close();
	        } catch (SQLException e) {
	          e.printStackTrace();
	        }
	      }
	      if (ps != null) {
	        try {
	          ps.close();
	        } catch (SQLException e) {
	          e.printStackTrace();
	        }
	      }
	      if (conn != null) {
	        try {
	          conn.close();
	        } catch (SQLException e) {
	          e.printStackTrace();
	        }
	      }
	    }
	    return stato;
	}

	@Override
	public void updateStato(Stato state) {
		
		Connection conn = null;
	    PreparedStatement ps = null;
	    ResultSet rs = null;
	     
	    String sql = "update eventi set capienza=? where id=?";
	     
	    try {
	      conn = db.getCon();
	      ps = conn.prepareStatement(sql);
	      ps.setInt(2, state.getId());
	      ps.setInt(1, state.getCapienza());
	    
	       
	      int rows = ps.executeUpdate();
	       
	      if (rows == 0) {
	        System.out.println("stato non creato");
	      }
	        
	    } catch (SQLException e) {
	      e.printStackTrace();
	    } finally {
	      if (rs != null) {
	        try {
	          rs.close();
	        } catch (SQLException e) {
	          
	          e.printStackTrace();
	        }
	      }
	      if (ps != null) {
	        try {
	          ps.close();
	        } catch (SQLException e) {
	        
	          e.printStackTrace();
	        }
	      }
	      if (conn != null) {
	        try {
	          conn.close();
	        } catch (SQLException e) {
	         
	          e.printStackTrace();
	        }
	      }
	    }
	}

	@Override
	public void createStato(Stato state) {
		Connection conn = null;
	    PreparedStatement ps = null;
	    ResultSet rs = null;
	     
	    String sql = "insert into eventi (id,capienza)"
	                + "values (?,?)";
	     
	    try {
	      conn = db.getCon();
	      ps = conn.prepareStatement(sql);
	      ps.setInt(1, state.getId());
	      ps.setInt(2, state.getCapienza());
	    
	       
	      int rows = ps.executeUpdate();
	       
	      if (rows == 0) {
	        System.out.println("stato non creato");
	      }
	        
	    } catch (SQLException e) {
	      e.printStackTrace();
	    } finally {
	      if (rs != null) {
	        try {
	          rs.close();
	        } catch (SQLException e) {
	          
	          e.printStackTrace();
	        }
	      }
	      if (ps != null) {
	        try {
	          ps.close();
	        } catch (SQLException e) {
	        
	          e.printStackTrace();
	        }
	      }
	      if (conn != null) {
	        try {
	          conn.close();
	        } catch (SQLException e) {
	         
	          e.printStackTrace();
	        }
	      }
	    }
	}
	private Database db=new Database();
	public static int capienza = 3;
}
