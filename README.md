    NodeMCU LUA Script to Post to AWS https://github.com/dsbaha/nodemcu-awsv4
    Copyright (C) 2015 Joseph Macaulay <joseph[dot]macaulay[at]gmail[dot]com>
  
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


HOWTO Use:

NODEMCU Prepartion:  rtctime.c was modified to add the rtctime.date() function.  Please replace the rtctime.c with the one included.  Please ensure that rtctime and sntp modules are compiled in and working properly.
```
aws = {}
aws.akId = "AMAZONKEYID"
aws.kSecret = "AMAZONSUPERSECRET"
aws.service = "sns" --service to connect to
aws.region = "us-west-1" --region to connect to
aws.signaturetype = "AWS4-HMAC-SHA256" --only one signature type supported
aws.req = {} --parameters of the request
aws.req.Message = "This is a Test Message"
aws.req.Subject = "This is a Test Subject"
aws.req.TopicArn = "arn:aws:" .. aws.service .. ":" .. aws.region .. ":089957316065:" .. wifi.sta.getmac():gsub('%W','')
aws.req.Action = "Publish"

awspoststring=dofile('awsv4.lua').post(aws)
```
This should generate an HTTP Post string like the following;
```
POST / HTTP/1.1
Authorization: AWS4-HMAC-SHA256 Credential=AMAZONKEYID/20151109/us-west-1/sns/aws4_request, SignedHeaders=content-type;host;x-amz-date, Signature=78f2f84d59ce6630ef5b0ff8d7ad0b80ff70aec96591db32efa0155df0bfc221
Host: sns.us-west-1.amazonaws.com
Content-Type: application/x-www-form-urlencoded
Content-Length: 133
X-Amz-Date: 20151109T203752Z

Message=This is a Test Message&Action=Publish&Subject=This is a Test Subject&TopicArn=arn:aws:sns:us-west-1:089957316065:18fe34a13b17
```

Then just use it like  ...

```
conn=net.createConnection(net.TCP,0)
conn:on('receive', function(conn, payload) print(payload) end)
conn:connect('80', aws.service .. '.' ..aws.region .. '.' .. 'amazonaws.com')
conn:send(awspoststring)
```
