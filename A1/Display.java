package internet;

public class Display{
  public synchronized void display(String message){
   for(int i=0;i<message.length();i++){
     char c = message.charAt(i);
     System.out.print(c);
     try{
       Thread.sleep(1);
     }catch(Exception e){
       System.out.println("Error sleeping");
     }
   }
 }
}
