# Sanitization fixture

<script>window.compromised = true</script>

<iframe src="https://example.test/frame"></iframe>

<a href="javascript:alert(1)" onclick="alert(2)">unsafe link</a>

<img src="images/local.png" srcset="https://example.test/tracker 2x" onerror="alert(3)" alt="local">

![Remote tracker](https://example.test/tracker.png)

