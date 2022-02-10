using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using uPLibrary.Networking.M2Mqtt.Messages;
using uPLibrary.Networking.M2Mqtt;
using System.Net.Security;
using System;
using System.Text;

public class MqttScript : MonoBehaviour
{

    private MqttClient client;
    public string brokerHostname = "192.168.0.162";
    public int brokerPort = 1883;
    static string subTopic = "#";
    private static string slideData;

    public void print()
    {
        Debug.Log("lal");
    }

    // Start is called before the first frame update
    void Start()
    {


        if (brokerHostname != null)
        {
            Debug.Log("connecting to " + brokerHostname + ":" + brokerPort);
            client = Connect();
            Subscribe(client, "chris/led");

        }
    }

    // Update is called once per frame
    void Update()
    {
        if (!client.IsConnected)
        {
            client = Connect();
            Subscribe(client, "#");
        }
    }

    public static MqttClient Connect()
    {

        MqttClient client = new MqttClient("192.168.0.162");
        string clientId = Guid.NewGuid().ToString();
        try
        {
            client.Connect(clientId);
        }
        catch (Exception e)
        {
            Debug.LogError("Connection error: " + e);
        }
        return client;

    }

    public static void Subscribe(MqttClient client, string topic)
    {
        client.MqttMsgPublishReceived += Client_MqttMsgPublishReceived;
        string clientId = Guid.NewGuid().ToString();
        client.Connect(clientId);
        client.Subscribe(new string[] { topic }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
    }

    public static void Client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
    {
        Debug.Log(e.Topic + " : " + Encoding.UTF8.GetString(e.Message));

        if ((Encoding.UTF8.GetString(e.Message)) != null)
        {
            slideData = Encoding.UTF8.GetString(e.Message);
        }

    }

    public string getData()
    {
        if(slideData != null)
        {
            return slideData;
        }
        else
        {
            return null;
        }
        
    }

    public static void Publish(MqttClient client, string title, string value)
    {
        string strValue = Convert.ToString(value);
        client.Publish(title, Encoding.UTF8.GetBytes(strValue));
    }



}
