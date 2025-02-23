<?php
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "election_db";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Fetch results
$sql = "SELECT candidate_name, votes, percentage, total_votes, winner_name, timestamp FROM results ORDER BY timestamp DESC LIMIT 3";
$result = $conn->query($sql);
?>

<!DOCTYPE html>
<html>
<head>
    <title>Election Results</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #f4f4f4; color: #333; text-align: center; }
        table { margin: 20px auto; border-collapse: collapse; width: 80%; }
        th, td { padding: 10px; border: 1px solid #ddd; }
        th { background-color: #007BFF; color: white; }
        tr:nth-child(even) { background-color: #f2f2f2; }
        h1 { color: #007BFF; }
    </style>
</head>
<body>
    <h1>Election Results</h1>
    <table>
        <tr>
            <th>Candidate</th>
            <th>Votes</th>
            <th>Percentage</th>
            <th>Total Votes</th>
            <th>Winner</th>
            <th>Timestamp</th>
        </tr>
        <?php
        if ($result->num_rows > 0) {
            while($row = $result->fetch_assoc()) {
                echo "<tr><td>" . $row["candidate_name"]. "</td><td>" . $row["votes"]. "</td><td>" . $row["percentage"]. "%</td><td>" . $row["total_votes"]. "</td><td>" . $row["winner_name"]. "</td><td>" . $row["timestamp"]. "</td></tr>";
            }
        } else {
            echo "<tr><td colspan='6'>No results found</td></tr>";
        }
        ?>
    </table>
</body>
</html>

<?php
$conn->close();
?>
