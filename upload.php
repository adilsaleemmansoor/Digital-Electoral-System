<?php
// Path to your results.json file
$jsonFilePath = 'results.json';

// Check if the request method is POST
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Get the JSON data from the POST request
    $jsonData = file_get_contents('php://input');

    // Decode the incoming JSON data to validate it
    $data = json_decode($jsonData, true);

    // Check if the JSON data is valid
    if ($data === null) {
        echo json_encode(['message' => 'Invalid JSON received.']);
        http_response_code(400); // Bad request
        exit;
    }

    // Option to either replace the file or append the new data
    $replace = true; // Set to false if you want to append

    if ($replace) {
        // Replace the existing results.json file with new data
        file_put_contents($jsonFilePath, json_encode($data, JSON_PRETTY_PRINT));
    } else {
        // Append the new data (merging the new data into the existing file)
        $existingData = json_decode(file_get_contents($jsonFilePath), true);
        $updatedData = array_merge($existingData, $data);
        file_put_contents($jsonFilePath, json_encode($updatedData, JSON_PRETTY_PRINT));
    }

    // Respond to the ESP32 or client that the operation was successful
    echo json_encode(['message' => 'Data successfully updated.']);
} else {
    // If the request is not POST, return a 405 Method Not Allowed
    http_response_code(405);
    echo json_encode(['message' => 'Method not allowed.']);
}
?>

