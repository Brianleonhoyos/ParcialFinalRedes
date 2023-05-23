document.getElementById("login-form").addEventListener("submit", function (event) {
  event.preventDefault();
  let username = document.getElementById("username").value;
  let password = document.getElementById("password").value;

  if (username === "admin" && password === "admin123") {
    // Redireccionar al panel de administrador
    window.location.href = "admin.html";
  } else if (username === "mantenimiento" && password === "mantenimiento123") {
    // Redireccionar al panel de personal de mantenimiento
    window.location.href = "mantenimiento.html";
  } else {
    alert("Credenciales incorrectas. Inténtalo de nuevo.");
  }
});