package Servlet;

import java.io.IOException;
import java.io.PrintWriter;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

public class Host extends HttpServlet {
    int subnetBits;
    int maskBits;
    int hostBits;
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {       
        response.setContentType("text/html;charset=UTF-8");
        HttpSession session = request.getSession();    
        //parameters
        String subnetSelected = request.getParameter("subnet");
        System.out.println(subnetSelected);
        //attributes
        String netClass = (String)session.getAttribute("netClass");
        String netMask = (String)session.getAttribute("netMask");
        int noSubnet = Integer.parseInt((String)session.getAttribute("noSubnet"));
        int noHost = Integer.parseInt((String)session.getAttribute("noHost"));
        String subnetMask = (String)session.getAttribute("subnetMask");
        String prefix = (String)session.getAttribute("prefix");
        String ipAddress = (String)session.getAttribute("ipAddress");
        //Number of bits used 
        subnetBits  = Integer.parseInt((String)session.getAttribute("subnetBits"));
        maskBits = Integer.parseInt((String)session.getAttribute("maskBits"));
        hostBits = Integer.parseInt((String)session.getAttribute("hostBits"));
        //example
        String subnet = "";
        String hostSubnet = "";
        

        try (PrintWriter out = response.getWriter()) {
            out.println("<!DOCTYPE html>");
            out.println("<html>");
            out.println("    <head>\n" +
            "        <title>Calculadora IP</title>\n" +
            "        <link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\">\n" +
            "        <meta charset=\"UTF-8\">\n" +
            "        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n" +
            "    </head>");
            out.println("<body>");
            out.println("        <h1>Calculadora IP</h1>\n" +
            "        <div class=\"row\">\n" +
            "            <div class=\"col-6\" id='info'>\n" +
            "                    <h2>Clase "+netClass+"</h2>\n" +
            "                    <p>Mascara de red: "+netMask+"</p>\n" +
            "                    <p>Subredes: "+noSubnet+"</p>\n" +
            "                    <p>Host: "+noHost+"</p>\n" +
            "                    <p>Mascara de subred: "+subnetMask+"</p>\n" +
            "            </div>\n" +
            "        </div>\n" +
            "        <div class=\"row\">\n" +
            "            <div class=\"col-6\">\n" +
            "                <div class=\"row\">\n" +
            "                    <h2>Lista de subredes</h2>\n" +
            "                </div>\n" +
            "                <div class=\"row\">\n" +
                    
            "                <form method='get' action='Host'>\n" +
            "                    <select name=\"subnet\" id=\"subnet\" size=\"10\">\n");
            
            for(int i=1;i<=noSubnet;i++){
                subnet = buildSubnet(ipAddress,i);
                out.println("<option value='"+subnet+"'>"+i+". "+subnet+"/"+prefix+"</option>");
            }
            out.println(
            "                    </select>\n" +
            "                    <input type='submit' value='mostrar'/>\n" +
            "                </form>\n" +
            "                </div>\n" +
            "            </div>\n" +
            "            <div class=\"col-6\">\n" +
            "                <div class=\"row\">\n" +
            "                    <h2 id=\"hostTitle\"></h2>\n" +
            "                </div>\n" +
            "                <div>\n" +
            "                    <ul id=\"list-style\">\n");
            
            for(int host=1;host<=noHost;host++){
                hostSubnet = buildHostSubnet(subnetSelected,host);
                out.println("<li>"+host+". "+hostSubnet+"</li>");
            }
            
            out.println(
            "                    </ul>\n" +
            "                </div>\n" +
            "            </div>\n" +
            "        </div>\n" +
            "        <script src=\"functions.js\"></script>");
            out.println("</body>");
            out.println("</html>");
    
        }
    }
    
    public String buildHostSubnet(String subnet,int host){
        long longHostSubnet = ipToLong(subnet)+host;
        String hostSubnet = longToIp(longHostSubnet);
        return hostSubnet;
    }
            
    public String buildSubnet(String ipAddress, int subnet){
        String strBin8bit = "",subnetAddress = "", strBin = "";
        String[] ipArray = new String[4]; 
        ipArray = ipAddress.split("\\.");
        int byteMask = maskBits/8;
        long longSubnet = 0;
        //build bit string (base, subnet + host)
        for(int i=0;i<byteMask;i++){
            strBin = Integer.toBinaryString(Integer.parseInt(ipArray[i]));
            strBin8bit += String.format("%8s", strBin).replaceAll(" ", "0");//integer to binary string (8 bit each)
        }
        for(int i=0;i<subnetBits;i++)
            strBin8bit+="0";
        longSubnet = Long.parseLong(strBin8bit, 2) + subnet; //from binary str to long
        longSubnet *= Math.pow(2,hostBits); //+ host
        subnetAddress = longToIp(longSubnet);//get subnet address
        
        return subnetAddress;
    }
    
    private long ipToLong(String ipAddress) {
        String[] addrArray = ipAddress.split("\\.");
        long num = 0;
        for (int i = 0; i < addrArray.length; i++) {
            int power = 3 - i;
            num += ((Integer.parseInt(addrArray[i]) % 256 * Math.pow(256, power)));
        }
        return num;
    }
    
    private String longToIp(long i) {
        return ((i >> 24) & 0xFF) + "." + ((i >> 16) & 0xFF) + "." + ((i >> 8) & 0xFF) + "." + (i & 0xFF);
    }
}