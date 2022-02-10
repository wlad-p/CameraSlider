using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Kamerahalterung : MonoBehaviour
{

    private MqttScript mqttScript;
    private string data;

    private int steps;
    private int dir;
    private int speed;
    private Vector3 current_pos;
    private string last_data;
    private Vector3 target_position;
    private bool lever = true;
    List<int[]> moves = new List<int[]>();
    private int range = 720;
    private int theoretical_pos = 10;

    // Start is called before the first frame update
    void Start()
    {
        mqttScript = FindObjectOfType<MqttScript>();

    }

    int[] read_command(string data)
    {
            string[] dataSplit = data.Split('/');
            steps = int.Parse(dataSplit[0]);
            dir = int.Parse(dataSplit[1].Substring(0, 1));
            speed = int.Parse(dataSplit[1].Substring(1));

            if (dir == 0)
            {
                dir = -1;
            }
            else
            {
                dir = 1;
            }

            //check if we go out of bounds
            steps = (steps / 10) * dir;
        print("Original steps: " + steps);
        if (this.theoretical_pos + steps >= this.range) { steps = this.range - this.theoretical_pos; }
        else if(this.theoretical_pos + steps < 10) { steps = (-1) * (int)transform.position.x+10;
            print("changed current position is: " + this.theoretical_pos);
        }
        print("Adapted steps: " + steps);
        print("Pos. " + this.theoretical_pos);

        this.theoretical_pos = this.theoretical_pos + steps;

        int[] vals = new int[3];
            vals[0] = steps;
            vals[1] = dir;
            vals[2] = speed;

            return vals;

    }


    void Update()
    {
        //print(this.current_pos);
        this.current_pos = transform.position;
        data = mqttScript.getData();
        if(data != null && data != this.last_data)
        {
            if(data == "rst")
            {
                //send command to reach the start
                int[] vals = read_command("10000/099");
                moves.Add(vals);
                return;
            }

            this.last_data = data;
            print(data);
            //check and cut head and tail
            if (data[0] == '[' || data[-1] == ']')
            {
                //remove head and tail
                data = data.TrimStart('[');
                data = data.TrimEnd(']');
                print(data);

                int index = data.IndexOf('[');
                int length_commands = int.Parse(data.Substring(0, index));
                print("Amount of commands: " + length_commands);
                //remove remaining head
                data = data.Remove(0, index+1);
                //string should be clean now
                print("Clean string:");
                print(data);

                int end = 0;
                int start = 0;

                for (int i = 0; i < length_commands; i++)
                {
                    print("Round number: " + i);
                    //index ;
                    int index_s = data.IndexOf(";", end+1);

                    //index !
                    if(end != 0) { start = end + 1; }
                    print("start is at: " + start);
                    end = data.IndexOf("!", end + 1);
                    print("end is at: " + end);
                    //size of single command
                    int size = index_s - start;

                    string command = data.Substring(start, size);

                    print(command);

                    int[] vals = read_command(command);
                    //alte Logik
                    print("Steps: " + vals[0]);
                    print("Dir: " + vals[1]);
                    print("Speed: " + vals[2]);
                    int xCoor = vals[2] * vals[1];

                    //Add move to array
                    this.moves.Add(vals);
                    
                }
            }



        }
        if(moves.Count != 0)
        {

            if (lever) { this.target_position = new Vector3(((float)moves[0][0]), 0, 0) + transform.position; print("Target is: " + this.target_position.x); this.lever = false; }
            
            
            transform.position = Vector3.MoveTowards(transform.position, this.target_position, 2*moves[0][2] * Time.deltaTime);

            this.current_pos = transform.position;
              //check if we reached our destination
            if(Vector3.Distance(transform.position, this.target_position) <= 0.001f || transform.position.x >= this.range || transform.position.x < 10)
            {
                this.moves.RemoveAt(0);
                this.lever = true;
                print("left Command");
            }
        }

    }
        


}
