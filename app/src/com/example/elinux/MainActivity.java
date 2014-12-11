package com.example.elinux;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

import android.widget.Button;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.view.View;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

public class MainActivity extends ActionBarActivity {

    private Socket socket;

    private static final int SERVERPORT = 1234;
    private static final String SERVER_IP = "192.168.42.2";
	
    private int pin = 0;

	private void setcolor(int color) {
        try {
            PrintWriter out = new PrintWriter(new BufferedWriter(
                    new OutputStreamWriter(socket.getOutputStream())),
                    true);
            out.println("rotate=0");
            out.println("p" + pin + "=" + Integer.toHexString(color));
            pin = (pin + 1) % 8;
        } catch (UnknownHostException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
	}

	private void rotate(int r) {
        try {
            PrintWriter out = new PrintWriter(new BufferedWriter(
                    new OutputStreamWriter(socket.getOutputStream())),
                    true);
            out.println("rotate=" + r);
            pin = 0;
        } catch (UnknownHostException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
	}
	
	private void rotatetime(int t) {
        try {
            PrintWriter out = new PrintWriter(new BufferedWriter(
                    new OutputStreamWriter(socket.getOutputStream())),
                    true);
            out.println("rotatetime=" + t);
        } catch (UnknownHostException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
	}
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        new Thread(new ClientThread()).start();
        
        final Button button01 = (Button) findViewById(R.id.Button01);
        button01.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                setcolor(0xff0000);
            }
        });

        final Button button02 = (Button) findViewById(R.id.Button02);
        button02.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	setcolor(0xffff00);
            }
        });

        final Button button03 = (Button) findViewById(R.id.Button03);
        button03.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	setcolor(0x00ff00);
            }
        });

        final Button button04 = (Button) findViewById(R.id.Button04);
        button04.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	setcolor(0x00ffff);
            }
        });

        final Button button05 = (Button) findViewById(R.id.Button05);
        button05.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	setcolor(0x0000ff);
            }
        });

        final Button buttonrotate0 = (Button) findViewById(R.id.buttonrotate0);
        buttonrotate0.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	rotate(0);
            }
        });

        final Button buttonrotate1 = (Button) findViewById(R.id.buttonrotate1);
        buttonrotate1.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	rotate(1);
            }
        });

        final Button buttonrotate2 = (Button) findViewById(R.id.buttonrotate2);
        buttonrotate2.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	rotate(2);
            }
        });

        final Button buttonrotate3 = (Button) findViewById(R.id.buttonrotate3);
        buttonrotate3.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	rotate(3);
            }
        });

        final SeekBar seekBar1 = (SeekBar) findViewById(R.id.seekBar1);
        seekBar1.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			int progressChanged = 0;

			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
				progressChanged = progress;
			}

			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
			}

			public void onStopTrackingTouch(SeekBar seekBar) {
				rotatetime(progressChanged);
			}
        });
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
    class ClientThread implements Runnable {
    	@Override
    	public void run() {
    		try {
        		InetAddress serverAddr = InetAddress.getByName(SERVER_IP);
        		socket = new Socket(serverAddr, SERVERPORT);
    		} catch (UnknownHostException e) {
    			e.printStackTrace();
    		} catch (IOException e) {
    			e.printStackTrace();
    		}
    	}
    };
}
