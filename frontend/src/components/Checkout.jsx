import { useState } from "react"

function Checkout({
  cart,
  currentUser,
  onClose,
  onPlaceOrder,
}) {
  const [address, setAddress] = useState("")
  const [paymentMethod, setPaymentMethod] = useState("Cash on Delivery")
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState("")

  const total = cart.reduce(
    (sum, item) => sum + item.price * item.quantity,
    0
  )

  const handleSubmit = async (event) => {
    event.preventDefault()

    setError("")

    if (!currentUser) {
      setError("Please login before placing an order.")
      return
    }

    if (!address.trim()) {
      setError("Please enter your delivery address.")
      return
    }

    if (cart.length === 0) {
      setError("Your cart is empty.")
      return
    }

    setLoading(true)

    try {
      const response = await fetch(
        "http://localhost:18080/api/orders",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify({
            userId: currentUser.id,
            address: address.trim(),
            paymentMethod,
            items: cart.map((item) => ({
              id: item.id,
              name: item.name,
              price: item.price,
              quantity: item.quantity,
            })),
          }),
        }
      )

      const data = await response.json()

      if (!response.ok) {
        throw new Error(
          data.message || "Failed to place order"
        )
      }

      onPlaceOrder(data.order)

    } catch (error) {
      setError(error.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

      <div className="bg-white rounded-2xl shadow-xl w-full max-w-2xl p-8 max-h-[90vh] overflow-y-auto">

        <div className="flex justify-between items-center mb-6">

          <h2 className="text-3xl font-bold text-gray-800">
            Checkout
          </h2>

          <button
            onClick={onClose}
            className="text-gray-500 text-2xl hover:text-gray-800"
          >
            ×
          </button>

        </div>

        <form onSubmit={handleSubmit}>

          <h3 className="text-xl font-bold text-gray-800 mb-4">
            Delivery Address
          </h3>

          <textarea
            value={address}
            onChange={(event) =>
              setAddress(event.target.value)
            }
            placeholder="Enter your complete delivery address"
            rows="3"
            className="w-full border rounded-lg px-4 py-3 mb-6"
          />

          <h3 className="text-xl font-bold text-gray-800 mb-4">
            Payment Method
          </h3>

          <div className="grid grid-cols-2 gap-3 mb-6">

            {[
              "Cash on Delivery",
              "Card",
              "UPI",
              "Wallet",
            ].map((method) => (

              <label
                key={method}
                className={`border rounded-lg p-4 cursor-pointer ${
                  paymentMethod === method
                    ? "border-orange-500 bg-orange-50"
                    : "border-gray-300"
                }`}
              >

                <input
                  type="radio"
                  name="payment"
                  value={method}
                  checked={paymentMethod === method}
                  onChange={(event) =>
                    setPaymentMethod(event.target.value)
                  }
                  className="mr-2"
                />

                {method}

              </label>

            ))}

          </div>

          <h3 className="text-xl font-bold text-gray-800 mb-4">
            Order Summary
          </h3>

          <div className="space-y-3">

            {cart.map((item) => (

              <div
                key={item.id}
                className="flex justify-between border-b pb-2"
              >

                <span>
                  {item.name} × {item.quantity}
                </span>

                <span className="font-semibold">
                  ₹{item.price * item.quantity}
                </span>

              </div>

            ))}

          </div>

          <div className="flex justify-between text-2xl font-bold mt-6">

            <span>Total</span>

            <span className="text-orange-600">
              ₹{total}
            </span>

          </div>

          {error && (
            <div className="bg-red-100 text-red-700 px-4 py-3 rounded-lg mt-5">
              {error}
            </div>
          )}

          <button
            type="submit"
            disabled={loading}
            className="w-full bg-orange-600 text-white py-4 rounded-lg
                       font-bold mt-6 hover:bg-orange-700
                       disabled:bg-gray-400"
          >
            {loading ? "Placing Order..." : "Place Order"}
          </button>

        </form>

      </div>

    </div>
  )
}

export default Checkout