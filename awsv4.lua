local M

do

  local getdatetime = function()
    local tdatetime = {}
    tdatetime = rtctime.date()
    return string.format("%04d%02d%02dT%02d%02d%02dZ", tdatetime.year, tdatetime.month, tdatetime.day, tdatetime.hour, tdatetime.min, tdatetime.sec) 
  end

  local datetime = getdatetime()

  -- key=value&key=value from table
  local requeststring = function(awsTable)
    local buildstring
    for k,v in pairs(awsTable.req)
      do
        buildstring = buildstring and (buildstring .. "&" .. k .. "=" .. v) or (k .. "=" .. v)
      end
    return buildstring
  end

  -- service, region, requeststring
  local canonicalform = function(awsTable)
    return "POST\n/\n\ncontent-type:application/x-www-form-urlencoded\nhost:"
    .. awsTable.service .. "." .. aws.region .. ".amazonaws.com\n"
    .. "x-amz-date:" .. datetime .. "\n\ncontent-type;host;x-amz-date\n"
    .. crypto.toHex(crypto.hash("SHA256", awsTable.requeststring))
  end

  -- signaturetype, region, service, canonicalform
  local stringtosign = function(awsTable)
    return awsTable.signaturetype .. "\n"
    .. datetime .. "\n"
    .. string.sub(datetime, 0, 8) .. "/"
    .. awsTable.region .. "/"
    .. awsTable.service .. "/"
    .. "aws4_request\n"
    .. crypto.toHex(crypto.hash("SHA256", awsTable.canonicalform))
  end

  -- region, service, stringtosign
  local signature = function(awsTable)
    local awsv4signature
    awsv4signature = crypto.hmac("SHA256", string.sub(datetime, 0, 8), "AWS4" .. awsTable.kSecret)
    awsv4signature = crypto.hmac("SHA256", awsTable.region, awsv4signature)
    awsv4signature = crypto.hmac("SHA256", awsTable.service, awsv4signature)
    awsv4signature = crypto.hmac("SHA256", "aws4_request", awsv4signature)
    return crypto.toHex(crypto.hmac("SHA256", awsTable.stringtosign, awsv4signature))
  end

  -- signaturetype, akId, region, service, signature
  local authorization = function(awsTable)
    return "Authorization: " .. aws.signaturetype
    .. " Credential=" .. aws.akId .. "/"
    .. string.sub(datetime, 0, 8) .. "/"
    .. awsTable.region .. "/"
    .. awsTable.service .. "/"
    .. "aws4_request, SignedHeaders=content-type;host;x-amz-date, Signature="
    .. awsTable.signature
  end

  local poststring = function(awsTable)
    return "POST / HTTP/1.1\n"
    .. awsTable.authorization .. "\n"
    .. "Host: " .. awsTable.service .. "." .. awsTable.region .. ".amazonaws.com\n"
    .. "Content-Type: application/x-www-form-urlencoded\n"
    .. "Content-Length: " .. string.len(awsTable.requeststring) .. "\n"
    .. "X-Amz-Date: " .. datetime .. "\n\n"
    .. awsTable.requeststring

  end

  local post = function(awsTable)
    local tempTable = awsTable
    tempTable.requeststring = requeststring(tempTable)
    tempTable.canonicalform = canonicalform(tempTable)
    tempTable.stringtosign = stringtosign(tempTable)
    tempTable.signature = signature(tempTable)
    tempTable.authorization = authorization(tempTable)
    return poststring(tempTable)
  end

  -- expose
  M = {
    getdatetime = getdatetime,
    requeststring = requeststring,
    canonicalform = canonicalform,
    stringtosign = stringtosign,
    signature = signature,
    authorization = authorization,
    poststring = poststring,
    post = post,
  }

end
return M

