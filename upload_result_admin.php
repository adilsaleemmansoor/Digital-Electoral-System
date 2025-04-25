<?php
// Database configuration
$host = 'localhost'; // Your database host
$db_name = 'election'; // Your database name
$username = 'your_database_username'; // Your database username
$password = 'your_database_password'; // Your database password

// Create connection
$conn = new mysqli($host, $username, $password, $db_name);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Check if the request method is POST
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Get the user ID and other result data
    $user_id = $_POST['user_id'];
    $pti_votes = $_POST['PTI_votes'];
    $ppp_votes = $_POST['PPP_votes'];
    $anp_votes = $_POST['ANP_votes'];
    $total_votes = $_POST['TotalVotes'];
    $winner = $_POST['Winner'];

    // Initialize response array
    $response = array();

    // Process the uploaded image
    if (isset($_POST['image'])) {
        // Decode the base64 string
        $image_data = $_POST['image'];
        $image_data = str_replace('data:image/png;base64,', '', $image_data);
        $image_data = str_replace(' ', '+', $image_data);
        $decoded_image = base64_decode($image_data);

        // Specify the upload directory
        $upload_dir = 'uploads/'; // Make sure this directory exists and is writable
        $image_name = uniqid() . '.png'; // Generate a unique name for the image
        $image_path = $upload_dir . $image_name;

        // Save the image to the server
        if (file_put_contents($image_path, $decoded_image)) {
            // Prepare SQL statement to insert the result data
            $stmt = $conn->prepare("INSERT INTO election_results (user_id, PTI_votes, PPP_votes, ANP_votes, TotalVotes, Winner, image_path) VALUES (?, ?, ?, ?, ?, ?, ?)");
            $stmt->bind_param("iiiiiss", $user_id, $pti_votes, $ppp_votes, $anp_votes, $total_votes, $winner, $image_path);

            // Execute the query
            if ($stmt->execute()) {
                $response['success'] = true;
                $response['message'] = "Result and image uploaded successfully!";
            } else {
                $response['success'] = false;
                $response['message'] = "Failed to store results: " . $stmt->error;
            }

            // Close the statement
            $stmt->close();
        } else {
            $response['success'] = false;
            $response['message'] = "Failed to save image.";
        }
    } else {
        $response['success'] = false;
        $response['message'] = "No image uploaded.";
    }
} else {
    $response['success'] = false;
    $response['message'] = "Invalid request method.";
}

// Close the database connection
$conn->close();

// Send the response back to the client
header('Content-Type: application/json');
echo json_encode($response);
?>
