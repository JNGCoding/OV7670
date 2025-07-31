package org.example;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.concurrent.atomic.AtomicInteger;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextPane;
import javax.swing.WindowConstants;

public class Main {
    public static final BufferedImage paint = new BufferedImage(320, 240, BufferedImage.TYPE_INT_ARGB);
    public static final JLabel canvas = new JLabel(new ImageIcon(paint));
    public static final JTextPane jTextPane = new JTextPane();
    public static JScrollPane LogArea;
    public static String CAMERA_IP = null;
    public static Socket socket; // Connect to Port 80
    public final static int port = 80;
    public final static int IMAGE_COMMAND = 0x01;
    public final static int LOG_COMMAND = 0x02;
    public final static int ONLINE_HALT_COMMAND = 0x03;

    public static int[] calculate_alignment_coefficient(int scr_w, int scr_h, int r_w, int r_h, int r_x, int r_y) {
        int center_x = (r_w / 2) + r_x;
        int center_y = (r_h / 2) + r_y;
        int scr_center_x = scr_w / 2;
        int scr_center_y = scr_h / 2;

        return new int[]{scr_center_x - center_x, scr_center_y - center_y};
    }

    public static void addLog(String log) {
        System.out.println("Log -> " + log);

        if (!log.endsWith("\n")) {
            log += "\n";
        }

        jTextPane.setText(jTextPane.getText() + log);
        jTextPane.setCaretPosition(jTextPane.getDocument().getLength());
    }

    public static void updateCanvas(int[] image) {
        new Thread(() -> {
            Graphics2D g = paint.createGraphics();
            int byte_counter = 0;
            for (int y = 0; y < 240; y++) {
                for (int x = 0; x < 320; x++) {
                    int Color_byte = image[byte_counter++];
                    g.setColor(new Color(Color_byte, Color_byte, Color_byte));
                    g.drawLine(319 - x, 239 - y, 319 - x, 239 - y);
                }
            }
            canvas.repaint();
        }).start();
    }

    public static void main(String[] args) throws IOException, InterruptedException {
        main_activity_grayscale();
    }

    public static void main_activity_grayscale() throws IOException, InterruptedException {
        JFrame jFrame = new JFrame("Image Display");
        jFrame.setResizable(false);
        jFrame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        jFrame.setSize(400, 450);

        JPanel jPanel = new JPanel(null);
        jPanel.setBackground(Color.lightGray);
        jPanel.setBorder(BorderFactory.createEmptyBorder());

        int[] center_pos = calculate_alignment_coefficient(380, 280, 320, 240, 0, 0);
        canvas.setBounds(center_pos[0], 0, 320, 240);

        jTextPane.setBounds(0, 240, 400, 210);
        jTextPane.setEditable(false);


        LogArea = new JScrollPane(jTextPane);
        LogArea.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
        LogArea.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

        LogArea.setBounds(0, 240, 400, 210);

        jPanel.add(canvas);
        jPanel.add(LogArea);

        int[] buffer = new int[76800];
        final AtomicInteger buffer_pointer = new AtomicInteger(0);
        socket = new Socket("192.168.4.1", 80);  // Connect to Port 80

        InputStream sock_input = socket.getInputStream();
        OutputStream sock_output = socket.getOutputStream();

        // sock_output.write(0); // Force Connection.

        Thread.sleep(100);

        Thread readThread = new Thread(
                () -> {
                    try {
                        while (true) {
                            if (sock_input.available() > 0) {
                                int data = sock_input.read();

                                if (data == IMAGE_COMMAND) {
                                    System.out.println("Command == IMAGE_COMMAND");
                                    while (true) {
                                        if (sock_input.available() > 0) {
                                            int pixel = sock_input.read();
                                            buffer[buffer_pointer.get()] = pixel;
                                            // System.out.println(buffer_pointer.get() + ") Byte - " + pixel);
                                            buffer_pointer.set(buffer_pointer.get() + 1);
                                        }

                                        if (buffer_pointer.get() > 76800 - 1) {
                                            buffer_pointer.set(0);
                                            updateCanvas(buffer);

                                            break;
                                        }
                                    }
                                }

                                if (data == LOG_COMMAND) {
                                    System.out.println("Command == LOG_COMMAND");
                                    StringBuilder log = new StringBuilder();
                                    while (true) {
                                        if (sock_input.available() > 0) {
                                            char character = (char) sock_input.read();
                                            if (character == 0xFF) {
                                                break;
                                            } else {
                                                log.append(character);
                                            }
                                        }
                                    }
                                    addLog(log.toString());
                                }

                                if (data == ONLINE_HALT_COMMAND) {
                                    System.out.println("Command == ONLINE_HALT_COMMAND");
                                    addLog("Camera switched to Offline Mode.");
                                    addLog("Closed Socket Object and I/O Streams.");
                                    socket.close();
                                    sock_input.close();
                                    sock_output.close();
                                }
                            }
                        }
                    } catch (Exception e) {
                        System.out.println(e.getMessage());
                    }
                }
        );

        jFrame.addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent event) {
                super.windowClosing(event);
                try {
                    socket.close();
                    sock_input.close();
                    sock_output.close();
                } catch (Exception e) {
                    System.out.println(e.getMessage());
                }
            }
        });

        readThread.setDaemon(true);
        readThread.start();

        jFrame.add(jPanel);
        jFrame.setVisible(true);
    }
}
