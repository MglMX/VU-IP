package internet;

public class Syn1 extends Thread{

  private String message;
  private Display d;

  public void run(){
    for(int i=0;i<10;i++){
      d.display(this.message);
    }
  }

  Syn1(Display d, String m){
    this.message = m;
    this.d = d;
  }

  public static void main(String[] args){
    Display d = new Display();
    Syn1 synEn = new Syn1(d,"Helloword\n");
    Syn1 synFr = new Syn1(d,"Bonjour monde\n");

    synEn.start();
    synFr.start();

  }
}
