use reqwest;
use http::{HeaderMap};
use serde::{Serialize, Deserialize};
use std::{cmp::Ordering, str::FromStr};
use chrono::{DateTime, Duration, Local};
use lettre::{Message, SmtpTransport, Transport, message::Mailbox};
use lettre::transport::smtp::authentication::Credentials;
use std::env;

#[derive(Serialize, Deserialize)]
struct Entity {
    last_reported: String
}

#[tokio::main]
async fn main() {
    const URL: &str = "http://192.168.0.25:8123/api/states/sensor.health";
    let token: &str = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI4MzgyOTFhNDY2YjQ0YTU3OTcxNDFjZDQ1YjI2Mzg5ZSIsImlhdCI6MTc3ODY3Njg3MywiZXhwIjoyMDk0MDM2ODczfQ.vHvX8TvnxdD9QNZ5kQ3og1qxukp_Nuugl1GLsRE-NIY";
    const MAIL_PASSWORD_KEY: &str= "MAIL_PASSWORD_KEY";
    let mail_password: String;

    match env::var(MAIL_PASSWORD_KEY){
        Ok(val) => mail_password = val,
        Err(_e) => panic!("Mail password not set"),
    }

    let client = reqwest::Client::new();

    let mut header: HeaderMap = http::HeaderMap::new();
    header.insert("content-type", http::HeaderValue::from_static("application/json"));
    let get_result =  client.get(URL)
        .headers(header)
        .bearer_auth(token)
        .send().await;

    let mut last_reported: Option<DateTime<Local>> = None;
    let current_time: DateTime<Local> = Local::now();
    
    match get_result{
        Ok(value) => {
            let text_response = value.text().await.unwrap();
            let entity : Entity = serde_json::from_str(&text_response).unwrap();
            last_reported = Option::Some(chrono::DateTime::from_str(&entity.last_reported).unwrap());
        },
        Err(err) => println!("error: {err}"),
    }
    // let text: DateTime<Local> = last_reported.unwrap();
    // println!("{text}");
    let mut error: &str = "";
    if last_reported.is_none()
    {
        error = "Unable to get last report";
    }
    else if last_reported.unwrap() + Duration::minutes(15) < current_time {
        error = "15 minutes without temperature update";
    }

    if error.cmp("") != Ordering::Equal{
        println!("{error}");
        let sender = Mailbox::new(Some("Server".to_owned()), "casasolaropaez@gmail.com".parse().unwrap());
        let receiver = Mailbox::new(Some("Jose Solaro".to_owned()), "josesolaro@gmail.com".parse().unwrap());
        let body = String::from(error);

        let email = Message::builder()
            .from(sender)
            .to(receiver)
            .subject("Falla en termotanque")
            .body(body)
            .unwrap();

        let creds = Credentials::new("casasolaropaez@gmail.com".to_string(), mail_password.to_string());
        let mailer = SmtpTransport::relay("smtp.gmail.com")
            .unwrap()
            .credentials(creds)
            .build();

        // match mailer.send(&email){
        //     Ok(_) => println!("Email sent"),
        //     Err(e) => println!("Could not sent: {:?}", e)
        // }

    }

    println!("finish");
}

