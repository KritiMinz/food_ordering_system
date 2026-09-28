import { useState } from "react"

function AuthModal({
  mode,
  onClose,
  onSwitchMode,
  onLoginSuccess,
}) {
  const isLogin = mode === "login"

  const [name, setName] = useState("")
  const [phone, setPhone] = useState("")
  const [email, setEmail] = useState("")
  const [password, setPassword] = useState("")
  const [confirmPassword, setConfirmPassword] = useState("")

  const [error, setError] = useState("")
  const [loading, setLoading] = useState(false)

  const handleSubmit = async (event) => {
    event.preventDefault()

    setError("")

    if (!isLogin && password !== confirmPassword) {
      setError("Passwords do not match")
      return
    }

    setLoading(true)

    try {
      const endpoint = isLogin
        ? "http://localhost:18080/api/login"
        : "http://localhost:18080/api/register"

      const body = isLogin
        ? {
            email,
            password,
          }
        : {
            name,
            phone,
            email,
            password,
          }

      const response = await fetch(endpoint, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(body),
      })

      const data = await response.json()

      if (!response.ok) {
        throw new Error(data.message || "Authentication failed")
      }

      if (isLogin) {
        localStorage.setItem("authToken", data.token)
        localStorage.setItem("currentUser", JSON.stringify(data.user))

        onLoginSuccess(data.user)
        onClose()
      } else {
        alert("Registration successful. Please login.")

        onSwitchMode()
      }
    } catch (error) {
      setError(error.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

      <div className="bg-white rounded-2xl shadow-xl w-full max-w-md p-8">

        <div className="flex justify-between items-center mb-6">

          <h2 className="text-3xl font-bold text-gray-800">
            {isLogin ? "Welcome Back" : "Create Account"}
          </h2>

          <button
            onClick={onClose}
            className="text-gray-500 text-2xl hover:text-gray-800"
          >
            ×
          </button>

        </div>

        <form onSubmit={handleSubmit}>

          {!isLogin && (
            <>
              <label className="block text-sm font-semibold text-gray-700 mb-2">
                Full Name
              </label>

              <input
                type="text"
                value={name}
                onChange={(e) => setName(e.target.value)}
                placeholder="Enter your name"
                required
                className="w-full border rounded-lg px-4 py-3 mb-4"
              />

              <label className="block text-sm font-semibold text-gray-700 mb-2">
                Phone
              </label>

              <input
                type="tel"
                value={phone}
                onChange={(e) => setPhone(e.target.value)}
                placeholder="Enter phone number"
                required
                className="w-full border rounded-lg px-4 py-3 mb-4"
              />
            </>
          )}

          <label className="block text-sm font-semibold text-gray-700 mb-2">
            Email
          </label>

          <input
            type="email"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
            placeholder="Enter your email"
            required
            className="w-full border rounded-lg px-4 py-3 mb-4"
          />

          <label className="block text-sm font-semibold text-gray-700 mb-2">
            Password
          </label>

          <input
            type="password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            placeholder="Enter password"
            required
            minLength="6"
            className="w-full border rounded-lg px-4 py-3 mb-4"
          />

          {!isLogin && (
            <>
              <label className="block text-sm font-semibold text-gray-700 mb-2">
                Confirm Password
              </label>

              <input
                type="password"
                value={confirmPassword}
                onChange={(e) => setConfirmPassword(e.target.value)}
                placeholder="Confirm password"
                required
                className="w-full border rounded-lg px-4 py-3 mb-5"
              />
            </>
          )}

          {error && (
            <div className="bg-red-100 text-red-700 px-4 py-3 rounded-lg mb-4">
              {error}
            </div>
          )}

          <button
            type="submit"
            disabled={loading}
            className="w-full bg-orange-600 text-white py-3 rounded-lg
                       font-semibold hover:bg-orange-700
                       disabled:bg-gray-400"
          >
            {loading
              ? "Please wait..."
              : isLogin
                ? "Login"
                : "Create Account"}
          </button>

        </form>

        <p className="text-center text-gray-500 mt-5">

          {isLogin
            ? "Don't have an account?"
            : "Already have an account?"}

          <button
            onClick={onSwitchMode}
            className="text-orange-600 font-semibold ml-2 hover:underline"
          >
            {isLogin ? "Register" : "Login"}
          </button>

        </p>

      </div>

    </div>
  )
}

export default AuthModal