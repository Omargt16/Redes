package Servlet;

import java.io.IOException;
import java.io.PrintWriter;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

public class GetipInfo extends HttpServlet {
    int freeBits = 0;
    int maskBits = 0;
    int subnetBits = 0;
    int hostBits = 0;
    String netMask = "";

    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        response.setContentType("text/html;charset=UTF-8");
        HttpSession session = request.getSession();        
        //Parameters
        String ipAddress = request.getParameter("ipAddress");
        String option = request.getParameter("option");
        String strValue = request.getParameter("value");
        int value = Integer.parseInt(strValue);
        //Calculate
        String netClass = getNetClass(ipAddress);//set freeBits and netMask
        int noSubnet = getSubnet(option,value);
        int noHost = getHost(option,value);
        String subnetMask = getSubnetMask();
        //more
        int prefix = maskBits + subnetBits;
        String subnet = "";

        session.setAttribute("ipAddress",ipAddress);
        session.setAttribute("netClass", netClass);
        session.setAttribute("netMask", netMask);
        session.setAttribute("noSubnet", Integer.toString(noSubnet));
        session.setAttribute("noHost", Integer.toString(noHost));
        session.setAttribute("subnetMask", subnetMask);
        session.setAttribute("prefix", Integer.toString(prefix));
        session.setAttribute("subnetBits",Integer.toString(subnetBits));
        session.setAttribute("maskBits", Integer.toString(maskBits));
        session.setAttribute("hostBits", Integer.toString(hostBits));

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
            "            </div>\n" +
            "        </div>\n" +
            "        <script src=\"functions.js\"></script>");
            out.println("</body>");
            out.println("</html>");
        }
    }
    
    public String getNetClass(String ipAddress){
        String[] ipArray = new String[4]; 
        ipArray = ipAddress.split("\\.");
        int firstByte = Integer.parseInt(ipArray[0]);
        String netClass = "";
        
        if(firstByte>=1 && firstByte<=127){
            freeBits = 24;
            netMask = "255.0.0.0";
            netClass = "A";
        } 
        else if(firstByte>=128 && firstByte<=191){
            freeBits = 16;
            netMask = "255.255.0.0";
            netClass = "B";
        } 
        else{
            freeBits = 8;
            netMask = "255.255.255.0";
            netClass = "C";
        } 
        maskBits = 32-freeBits;
        return netClass;
    }
        
    public int getSubnet(String option, int value){
        int noSubnet = 0;
        int max = (int)Math.pow(2,freeBits)-2; 

        if(option.equals("host"))
            subnetBits = freeBits - getNoBits(max,value);
        else if(option.equals("prefijo"))
            subnetBits = value - maskBits;
        else //subnet
            subnetBits = getNoBits(max,value);
        
        noSubnet = (int)Math.pow(2,subnetBits)-2;
        
        return noSubnet;
    }
    
    public int getHost(String option, int value){
        int noHost = 0;
        int max = (int)Math.pow(2,freeBits)-2; 

        if(option.equals("host"))
            hostBits = getNoBits(max,value);
        else if(option.equals("prefijo"))
            hostBits = 32-value;
        else //subnet
            hostBits = freeBits - getNoBits(max,value);
        
        noHost = (int)Math.pow(2,hostBits)-2;
        
        return noHost;
    }
    
    private int getNoBits(int max, int value){ //how many bits do I require to represent an integer?
        int integer = 0;
        int bitsRequired = 1;
        
        while(!(integer>=value) && integer<= max){
            bitsRequired++;
            integer = (int)Math.pow(2,bitsRequired)-2;
        };
        
        return bitsRequired;
    }
    
    public String getSubnetMask(){
        int i = 0, eighthBitValue = 128, sum = 0;
        int maskBytes = maskBits/8;
        int subnetBytes = subnetBits/8;
        int remainingSubnetBits = subnetBits - 8*subnetBytes;
        String[] subnetMask = new String[4]; 

        while(maskBytes>=1){ //initial network mask
            subnetMask[i]="255";
            maskBytes--;
            i++;
        }

        for(int j=0;j<subnetBytes;j++){ //completed bytes
            subnetMask[i]="255"; i++;
        }
        
        if(remainingSubnetBits>0){ //remaining subnet values    
            while(remainingSubnetBits>0){
                sum+=eighthBitValue;
                eighthBitValue/=2;
                remainingSubnetBits--;
            }
            subnetMask[i] = Integer.toString(sum);
            i++;
        }
        
        while(i<4){ //0's
            subnetMask[i]="0"; i++;
        }
        
        return buildAddress(subnetMask);
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
    
    //aux
    private String longToIp(long i) {
        return ((i >> 24) & 0xFF) + "." + ((i >> 16) & 0xFF) + "." + ((i >> 8) & 0xFF) + "." + (i & 0xFF);
    }
    
    private String buildAddress(String[] ip){
        String address="";
        for(int i=0;i<3;i++)
            address+=ip[i]+".";
        address+=ip[3];
            
        return address;
    }
}
